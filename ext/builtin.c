#include "../helper.h"

simple_ctor(BUF, Delim, 2, (2, BUF, BUF), (1, BUF)) {
    bind_arg(0, Buf, under);
    bind_arg(1, Buf, delim);

    sz k;
    for (k = 0; k < under->len - delim->len; k++) {
        bool diff = memcmp(under->ptr+k, delim->ptr, delim->len);
        if (!diff) break;
    }

    self->as.buf.ptr = under->ptr;
    self->as.buf.len = k;
    return true;
}

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
