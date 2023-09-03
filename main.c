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
            printf("WARN: could not load '%s'\n", *it);
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
                    printf("ERROR: expected ext name\n");
                    return 1;
                }
                if (!exts_load(*++argv))
                    printf("WARN: could not load '%s'\n", *argv);
                break;

            default:
                printf("WARN: unknown flag '%s'\n", arg);
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

                                line[strlen(line+2)-1] = '\0';
                                Meta* meta = exts_lookup(mksym(line+2));
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

                default:
                    line[strlen(line+1)-1] = '\0';
                    obj_show(scope_get(&scope, mksym(line+1)), 0);
            }
        }

        else if (!lang_process(line, &scope)) {
            printf("\n");
        }
    }

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
