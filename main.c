#include "base.h"
#include "exts.h"
#include "lang.h"

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
        printf("%zu name-s:\n", exts_scope.count);
        for (sz k = 0; k < exts_scope.count; k++)
            printf("- %p: (%s) %s\n",
                    (void*)exts_scope.items[k].value,
                    ty_str[exts_scope.items[k].value->ty],
                    exts_scope.items[k].key.ptr);
    }

    else {
        for (; argc--; argv++) {
            Obj* it = exts_lookup(*argv);
            if (!it) printf("- %s not found\n", *argv);
            else printf("- %p: (%s) %s\n", (void*)it, ty_str[it->ty], *argv);
        }
    }

    exts_unload();
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

    return 0;
}

#if 0

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
