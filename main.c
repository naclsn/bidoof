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
                printf("Usage: %s ...\n", prog);
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

void repl(void) {
    Scope scope = {0};

    char line[256];

    while (printf(">> "), fgets(line, 256, stdin)) {
        if ('?' == line[0]) {
            switch (line[1]) {
                case '\n':
                    scope_show(&scope);
                    break;

                case '?':
                    switch (line[2]) {
                        case '\n':
                            scope_show(&exts_scope);
                            break;

                        default:
                            {
                                static char const* const ty_str[] = {[BUF]= "Buf", [NUM]= "Num", [LST]= "Lst", [FUN]= "Fun", [SYM]= "Sym"};

                                line[strlen(line+2)+1] = '\0';
                                Meta* meta = exts_lookup(mksym(line+2));
                                if (!meta) {
                                    puts("(null)");
                                    break;
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
                    }
                    break;

                case '%': {
                    line[strlen(line+2)+1] = '\0';
                    Obj const* it = scope_get(&scope, mksym(line+2));
                    if (it) switch (it->ty) {
                        case BUF: printf("\"%.*s\"\n", (int)it->as.buf.len, it->as.buf.ptr); break;
                        case NUM: printf("%d\n", it->as.num.val); break;
                        case SYM: printf(":%s\n", it->as.sym.txt); break;
                        default: printf("%p\n", it->as.lst.ptr);
                    } else puts("(null)");
                } break;

                default:
                    line[strlen(line+1)] = '\0';
                    obj_show(scope_get(&scope, mksym(line+1)), 0);
            }
        } // if '?'

        else if ('.' == line[0]) {
            if (0 == strcmp(".exit\n", line) || 0 == strcmp(".quit\n", line))
                break;
            if (0 == strcmp(".help\n", line))
                puts(
                    "cli commands:\n"
                    "  ?[name]\n"
                    "  ?%[name]\n"
                    "  ??[name]\n"
                    "  .exit\n"
                    "  .quit\n"
                    "  .help\n"
                );
        }

        else if (!lang_process("<repl_line>", line, &scope)) {
            printf("\n");
        }
    } // while ">> " fgets

    scope_show(&scope);
    scope_clear(&scope);
}

int main(int argc, char** argv) {
    argc--;
    char* prog = *argv++;

    load_all_exts(prog);
    int r = parse_args(prog, argc, argv);
    if (r) return r;

    repl();

    return 0;
}
