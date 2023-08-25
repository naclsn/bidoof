#include "base.h"
#include "loader.h"

int main(int argc, char** argv) {
    if (1 == argc) {
        puts("missing ext name");
        return 1;
    }

    if (!load_names(argv[1])) {
        puts("could not :)");
        return 1;
    }

    unload_all();

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
    Obj* res = call(Delim, 2, res_args);
    if (!res) {
        puts("call failed");
        return 1;
    }

    bool yes = update(under);
    if (!yes) {
        puts("update failed");
        return 1;
    }

    printf("value of res: ");
    show(res);
    printf(" `-> '%.*s'\n", (int)res->as.buf.len, res->as.buf.ptr);

    Obj* ser_args[1] = {res};
    Obj* ser = call(Reverse, 1, ser_args);

    update(under);

    printf("value of ser: ");
    show(ser);
    printf(" `-> '%.*s'\n", (int)ser->as.buf.len, ser->as.buf.ptr);

    delete(ser);
    delete(res);

    return 0;
}

#endif
