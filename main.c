#include <stddef.h>

#ifndef EXTS_NAMES
#warning "no $EXTS_NAMES, there will be no function..."
#define EXTS_NAMES
#endif
static char* exts_names[] = {EXTS_NAMES NULL};
#undef EXTS_NAMES

#include "base.h"
#include "exts.h"
#include "lang.h"

int main(int argc, char** argv) {
    argc--;
    char* prog = *argv++;

    for (char** it = exts_names; *it; it++)
        if (!exts_load(*it))
            printf("WARN: could not load '%s'\n", *it);

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

    Scope scope = {0};

    char* line = NULL;
    sz len = 0;

    printf(">> ");
    while (-1 != getline(&line, &len, stdin)) {
        if ('?' == line[0]) {
            switch (line[1]) {
                case '\0':
                case '\n':
                    scope_show(&scope);
                    break;

                case '?':
                    scope_show(&exts_scope);
                    break;

                default:
                    obj_show(scope_get(&scope, (Sym){.ptr= line+1, .len= strlen(line+1)-1}), 0);
            }
        }

        else if (!lang_process(strdup(line), &scope)) {
            printf("\n");
        }

        printf(">> ");
    }

    free(line);

    scope_show(&scope);
    scope_clear(&scope);

    return 0;
}

#if 0
static char* ty_str[] = { [BUF]= "BUF", [NUM]= "NUM", [LST]= "LST", [FUN]= "FUN", [SYM]= "SYM" };

int main_lookup(int argc, char** argv) {
    if (0 == argc) {
        puts("missing ext name");
        return 1;
    }

    char* ext_name = *argv++;
    argc--;
    if (!exts_load(ext_name)) {
        printf("could not load '%s'\n", ext_name);
        return 1;
    }

    if (0 == argc) {
        scope_show(&exts_scope);
    }

    else {
        for (; argc--; argv++) {
            Obj* it = exts_lookup(*argv);
            if (!it) printf("- %s not found\n", *argv);
            else {
                printf("- %p: (%s) %s\n", (void*)it, ty_str[it->ty], *argv);
                obj_show(it, 0);
                obj_show_depnts(it, 0);
            }
        }
    }

    exts_unload();
    return 0;
}

int main_script(int argc, char** argv) {
    Scope scope = {0};

    if (0 == argc) {
        char* line = NULL;
        sz len = 0;
        printf(">> ");
        while (-1 != getline(&line, &len, stdin)) {
            lang_process(line, &scope);
            printf(">> ");
        }
        free(line);
    }

    else {
        for (; argc--; argv++)
            lang_process(*argv, &scope);
    }

    scope_show(&scope);

    scope_clear(&scope);
    return 0;
}

int main(int argc, char** argv) {
    argv++;
    argc--;

    if (0 == argc) {
        puts("no argument");
        return 0;
    }

    if (0 == strcmp("-l", *argv)) return main_lookup(--argc, ++argv);
    if (0 == strcmp("-s", *argv)) return main_script(--argc, ++argv);

    puts("unknown argument");
    return 1;
}

if 0

// YYY: temporary hack
#define DECLARE_ONLY 1
# include "ext/builtin.c"
#undef DECLARE_ONLY

int main1(int _argc, char** _argv) {
    (void)_argc;
    (void)_argv;

    Obj* under = &(Obj){.ty= BUF, .as.buf= {.ptr= (u8*)"this is test", .len= 12}};
    Obj* space = &(Obj){.ty= BUF, .as.buf= {.ptr= (u8*)" ", .len= 1}};

    Obj* res_args[2] = {under, space};
    Obj* res = obj_call(Delim, 2, res_args);
    if (!res) {
        puts("call failed");
        return 1;
    }

    bool yes = obj_update(under);
    if (!yes) {
        puts("update failed");
        return 1;
    }

    printf("value of res: ");
    obj_show(res, 0);
    printf(" `-> '%.*s'\n", (int)res->as.buf.len, res->as.buf.ptr);

    Obj* ser_args[1] = {res};
    Obj* ser = obj_call(Reverse, 1, ser_args);

    obj_update(under);

    printf("value of ser: ");
    obj_show(ser, 0);
    printf(" `-> '%.*s'\n", (int)ser->as.buf.len, ser->as.buf.ptr);

    obj_destroy(ser);
    obj_destroy(res);

    return 0;
}

#endif
