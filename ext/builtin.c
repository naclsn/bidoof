#include "../helper.h"

export_names
    ( "Count"
    , "Delim"
    );

ctor_simple(2, Count
        , "count elements of a list or bytes of a buffer that verify a predicate"
        , (2, NUM, _CountB, BUF, from, FUN, pred)
        , (2, NUM, _CountL, LST, from, FUN, pred)
        );
bool _CountB(Num* self, Buf const* const from, Fun const* const pred) {
    Obj* fun = frommember(pred, Obj, as);

    union { u8 _buf[sizeof(Obj) + 1*sizeof(Obj*)]; Obj _obj; } _r;
    Obj* r = &_r._obj;

    self->val = 0;
    for (sz k = 0; k < from->len; k++) {
        memset(r, 0, sizeof *r);
        r->argc = 1;
        r->argv[0] = &(Obj){.ty= NUM, .as.num.val= from->ptr[k]};

        if (!fun->as.fun.call(fun, r)) return false;
        bool (*up)(Obj*) = r->update;
        if (NUM != r->ty || (up && !up(r))) {
            r->update = NULL;
            if (up) up(r);
            return false;
        }

        if (0 != r->as.num.val) self->val++;

        r->update = NULL;
        if (up) up(r);
    }
    return true;
}
bool _CountL(Num* self, Lst const* const from, Fun const* const pred) {
    (void)from;
    (void)pred;
    self->val = 0;
    notify("NIY: streamline inline call process");
    return false;
}

ctor_simple(2, Delim
        , "slice from beginning up to delimiter (exclusive), or the whole buffer if not found - default delimiter is \"\\0\" ie. C-string"
        , (2, BUF, _Delim2, BUF, under, BUF, delim)
        , (1, BUF, _Delim1, BUF, under)
        );
bool _Delim2(Buf* self, Buf const* const under, Buf const* const delim) {
    sz k;
    for (k = 0; k < under->len - delim->len; k++) {
        if (0 == memcmp(under->ptr+k, delim->ptr, delim->len)) goto found;
    }
    k = under->len;
found:
    self->ptr = under->ptr;
    self->len = k;
    return true;
}
bool _Delim1(Buf* self, Buf const* const under) {
    self->ptr = under->ptr;
    for (self->len = 0; self->len < under->len && under->ptr[self->len]; self->len++);
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
