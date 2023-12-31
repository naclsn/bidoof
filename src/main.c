#define LINE_IMPLEMENTATION
#include "line.h"

#include <unistd.h> // YYY(temp): sleep

#include "exts.h"
#include "lang.h"

#ifndef EXTS_NAMES
#error "no $EXTS_NAMES, there would be no function..."
#define EXTS_NAMES
#endif
static char* exts_names[] = {EXTS_NAMES NULL};
#undef EXTS_NAMES

void load_all_exts(char const* prog) {
    char prog_dir[256];
    char* end = strrchr(strncpy(prog_dir, prog, 256), '/');

    for (char** it = exts_names; *it; it++) {
        sz len = strlen(*it);
        memcpy(end+1, *it, len);
        end[len+1] = '\0';

        if (!exts_load(prog_dir))
            notify_printf(24+len, "WARN: could not load '%s'", *it);
    }
}

int parse_args(char* prog, int argc, char** argv) {
    for (; argc--; argv++) {
        char* arg = *argv;
        if ('-' == arg[0]) switch (arg[1]) {
            case 'h':
                printf("Usage: %s [-l <lib>] ...\n", prog);
                return 1;

            case 'l':
                if (0 == --argc) {
                    notify("ERROR: expected ext name");
                    return 1;
                }
                if (!exts_load(*++argv))
                    notify_printf(24+strlen(*argv), "WARN: could not load '%s'", *argv);
                break;

            default:
                notify_printf(22+strlen(arg), "WARN: unknown flag '%s'", arg);
                return 1;
        }
    }

    return 0;
}

void show_meta(char* name, sz len) {
    // (trim)
    while (' ' == name[len-1]) name[--len] = '\0';

    static char const* const ty_str[] = {[BUF]= "Buf", [NUM]= "Num", [FLT]= "Flt", [LST]= "Lst", [FUN]= "Fun", [SYM]= "Sym", [ANY]= "Any"};

    Meta* meta = exts_lookup(mksym(name));
    if (!meta) {
        puts("(null)");
        return;
    }

    puts(meta->doc);

    for (struct MetaOvl const* ovl = meta->overloads; ovl->params; ovl++) {
        printf("   %s %s(", ty_str[ovl->ret], meta->name);

        struct MetaOvlPrm const* prm = ovl->params;
        printf("%s %s", ty_str[prm->ty], prm->name);
        while ((++prm)->name) printf(", %s %s", ty_str[prm->ty], prm->name);

        printf(")\n");
    }
}

static Scope repl_scope = {0};

char** compgen_words(char* line, sz point) {
    sz len = 0;
    char c;
    while (point && ( '_' == (c = line[point-1-len])
            || ('0' <= c && c <= '9')
            || ('A' <= c && c <= 'Z')
            || ('a' <= c && c <= 'z')
            )) len++;
    char* ptr = line+point-len;
    while (len && '0' <= *ptr && *ptr <= '9') len--, ptr++;

    Scope* search_in = !len || ('A' <= *ptr && *ptr <= 'Z')
        ? &exts_scope
        : &repl_scope
        ;

    char** r = calloc(search_in->count + 1, sizeof(char*));
    if (r) {
        sz head = 0;
        for (sz k = 0; k < search_in->count; k++) {
            Sym* it = &search_in->items[k].key;
            if (0 == memcmp(it->txt, ptr, len)) {
                char* word = malloc(17);
                if (!word) return r;
                memcpy(word, it->txt+len, 16);
                strcat(word, " ");
                r[head++] = word;
            } // if matches
        } // for names is scope
    } // if r
    return r;
}

void compgen_clean(char** words) {
    if (words) {
        for (char** it = words; *it; it++) free(*it);
        free(words);
    }
}

void repl(char* histfn) {
    {
        FILE* histf = fopen(histfn, "rb");
        if (histf) {
            fseek(histf, 0, SEEK_END);
            sz len = ftell(histf);
            fseek(histf, 0, SEEK_SET);

            char* txt = malloc(len);
            if (txt) {
                fread(txt, len, 1, histf);
                char* start = txt;

                char* lines[_hist_ln];
                sz count = 0;
                for (sz k = 0; count < _hist_ln && k < len; k++) {
                    if ('\n' == txt[k]) {
                        txt[k] = '\0';
                        lines[count++] = start;
                        start = txt+k+1;
                    }
                }

                line_histset(lines, count);
                free(txt);
            }
            fclose(histf);
        }
    }
    line_compgen(compgen_words, compgen_clean);

    char const* line;
    while (printf(">> "), line = line_read()) {
        sz len = strlen(line);
        char cpy[len+2];
        memcpy(cpy+1, line, len+1);
        cpy[0] = ' ';

        switch (line[0]) {
            // shorthand for assigning to _ and print/format _
            case '?':
                switch (line[1]) {
                    case '?':
                        if ('\0' == line[2]) scope_show(&exts_scope);
                        else show_meta(cpy+3, len-2);
                        continue;

                    case '\0':
                        scope_show(&repl_scope);
                        continue;
                }

                // fallthrough
            case '%':
                cpy[0] = '_';
                cpy[1] = '=';
                break;

            // REPL commands
            case '.':
                if (0 == strcmp(".exit", line)) goto exit_no_hist;
                if (0 == strcmp(".quit", line)) break;

                if (0 == strcmp(".help", line))
                    puts(
                        "cli commands:\n"
                        "  ?[name]\n"
                        "  ?%[name]\n"
                        "  ??[name]\n"
                        "  .exit\n"
                        "  .quit\n"
                        "  .help\n"
                        "  .sleep[sec]\n"
                        "  .source <file>\n"
                        "  .tokens <script>\n"
                    );

                else if (0 == memcmp(".sleep", line, 6)) {
                    unsigned int sec = 0;
                    char const* a = line+6;
                    for (; ' ' == *a || '\t' == *a; ++a);
                    for (; '0' <= *a && *a <= '9'; ++a) sec = sec*10 + (*a & 0xf);
                    sleep(sec);
                }

                else if (0 == memcmp(".source ", line, 8)) {
                    char const* filename = line+8;
                    FILE *f = fopen(filename, "rb");
                    if (!f) continue;
                    fseek(f, 0, SEEK_END);
                    size_t len = ftell(f);
                    char* script = NULL;
                    if (len) {
                        fseek(f, 0, SEEK_SET);
                        script = malloc(len);
                        if (script) {
                            fread(script, len, 1, f);
                            script[len-1] = '\0';
                        }
                    }
                    fclose(f);
                    if (script) lang_process(filename, script, &repl_scope);
                    else printf("could not read file '%s'\n", filename);
                }

                else if (0 == memcmp(".tokens ", line, 8)) {
                    char const* script = line+8;
                    lang_show_tokens("<repl_line>", script);
                }

                // case '.'
                continue;

        } // switch (line[0])

        lang_process("<repl_line>", cpy, &repl_scope);

        switch (line[0]) {
            case '?':
                obj_show(scope_get(&repl_scope, mksym("_")), 0);
                break;

            case '%': {
                Obj const* it = scope_get(&repl_scope, mksym("_"));
                switch (it->ty) {
                    case BUF: printf("\"%.*s\"\n", (int)it->as.buf.len, it->as.buf.ptr); break;
                    case NUM: printf("%ld\n", it->as.num.val); break;
                    case FLT: printf("%lf\n", it->as.flt.val); break;
                    case SYM: printf(":%s\n", it->as.sym.txt); break;
                    default: printf("%p\n", (void*)it->as.lst.ptr);
                }
            } break;
        }
    } // while ">> " line_read

    {
        FILE* histf = fopen(histfn, "wb");
        if (histf) {
            sz o = 0;
            char** lines = line_histget(&o);
            while (o--) {
                fputs(lines[o], histf);
                fputc('\n', histf);
            }
            fflush(histf);
            fclose(histf);
        }
    }
exit_no_hist:
    line_free();
    scope_clear(&repl_scope);
}

int main(int argc, char** argv) {
    argc--;
    char* prog = *argv++;

    load_all_exts(prog);
    int r = parse_args(prog, argc, argv);
    if (r) return r;

    {
        char histfn[256];
        char* end = strrchr(strncpy(histfn, prog, 256), '/');
        memcpy(end+1, ".line_hist", 10);
        end[11] = '\0';

        puts("type '.help' for a list of REPL-specific commands");
        repl(histfn);
    }

    exts_unload();

    return 0;
}
