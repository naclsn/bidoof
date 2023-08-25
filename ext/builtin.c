#include "base.h"

char* names[3] = {"Delim", "Reverse", NULL};

bool _updateDelim(Obj* self) {
    Buf const* under = &self->argv[0]->as.buf;
    Buf const* delim = &self->argv[1]->as.buf;

    sz k;
    for (k = 0; k < under->len - delim->len; k++) {
        bool diff = memcmp(under->ptr+k, delim->ptr, delim->len);
        if (!diff) break;
    }

    self->as.buf.ptr = under->ptr;
    self->as.buf.len = k;
    return true;
}

bool _makeDelim(Obj* self, Obj* res) {
    (void)self;

    if (2 != res->argc) return false;
    if (BUF != res->argv[0]->ty) return false;
    if (BUF != res->argv[1]->ty) return false;

    res->update = _updateDelim;
    res->ty = BUF;
    return true;
}

Obj Delim = {.ty= FUN, .as.fun.call= _makeDelim};

bool _updateReverse(Obj* self) {
    if (!self->update) {
        free(self->as.buf.ptr);
        self->as.buf.ptr = NULL;
        self->as.buf.len = 0;
        return true;
    }

    Buf const* under = &self->argv[0]->as.buf;

    self->as.buf.ptr = realloc(self->as.buf.ptr, under->len);
    if (!self->as.buf.ptr) return false;

    for (sz k = 0; k < under->len; k++)
        self->as.buf.ptr[k] = under->ptr[under->len-1 - k];

    self->as.buf.len = under->len;
    return true;
}

bool _makeReverse(Obj* self, Obj* res) {
    (void)self;

    if (1 != res->argc) return false;
    if (BUF != res->argv[0]->ty) return false;

    res->update = _updateReverse;
    res->ty = BUF;
    return true;
}

Obj Reverse = {.ty= FUN, .as.fun.call= _makeReverse};
