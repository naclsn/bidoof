#include "../helper.h"

export_names("Delim");

ctor_simple(2, Delim
        , "slice from beginning up to delimiter (exclusive), or the whole buffer if not found - default delimiter is \"\\0\" ie. C-string"
        , (2, BUF, _Delim2, BUF, under, BUF, delim)
        , (1, BUF, _Delim1, BUF, under)
        );

bool _Delim2(Obj* self, Buf* r, Buf const* const under, Buf const* const delim) {
    (void)self;
    sz k;
    for (k = 0; k < under->len - delim->len; k++) {
        if (0 == memcmp(under->ptr+k, delim->ptr, delim->len)) goto found;
    }
    k = under->len;
found:
    r->ptr = under->ptr;
    r->len = k;
    return true;
}

bool _Delim1(Obj* self, Buf* r, Buf const* const under) {
    (void)self;
    r->ptr = under->ptr;
    r->len = strnlen((char const*)under->ptr, under->len);
    return true;
}

#if 0
simple_ctor(LST, Map, 1, (2, FUN, LST)) {
    (void)self;
    puts("NIY: Map");
    return false;
}

simple_ctor(BUF, Reverse, 1, (1, BUF)) {
    if (!self->update) {
        free(self->as.buf.ptr);
        self->as.buf.ptr = NULL;
        self->as.buf.len = 0;
        return true;
    }

    bind_arg(0, Buf, under);

    self->as.buf.ptr = realloc(self->as.buf.ptr, under->len);
    if (!self->as.buf.ptr) return false;

    for (sz k = 0; k < under->len; k++)
        self->as.buf.ptr[k] = under->ptr[under->len-1 - k];

    self->as.buf.len = under->len;
    return true;
}

export_names("Map", "Delim", "Reverse");
#endif
