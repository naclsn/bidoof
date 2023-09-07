#include "../helper.h"

export_names
    ( "Count"
    , "Delim"
    , "Join"
    , "Len"
    , "Range"
    );

ctor_simple(2, Count
        , "count elements of a list or bytes of a buffer that verify a predicate"
        , (2, NUM, _CountB, BUF, from, FUN, pred)
        , (2, NUM, _CountL, LST, from, FUN, pred)
        );
bool _CountB(Num* self, Buf const* const from, Fun const* const pred) {
    Obj* predf = frommember(pred, Obj, as);
    self->val = 0;
    for (sz k = 0; k < from->len; k++) {
        Obj num = {.ty= NUM, .as.num.val= from->ptr[k]};
        inline_call_assign(NUM, res, predf, 1, &num);
            if (0 != res->as.num.val) self->val++;
        inline_call_cleanup(res);
    }
    return true;
}
bool _CountL(Num* self, Lst const* const from, Fun const* const pred) {
    Obj* predf = frommember(pred, Obj, as);
    self->val = 0;
    for (sz k = 0; k < from->len; k++) {
        inline_call_assign(NUM, res, predf, 1, from->ptr[k]);
            if (0 != res->as.num.val) self->val++;
        inline_call_cleanup(res);
    }
    return true;
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

ctor_simple(2, Join
        , "join with separator - default delimiter is \"\" (empty)"
        , (2, BUF, _Join2, LST, list, BUF, sep)
        , (1, BUF, _Join1, LST, list)
        );
bool _Join2(Buf* self, Lst const* const list, Buf const* const sep) {
    if (destroyed(self)) {
        free(self->ptr);
        self->ptr = NULL;
        self->len = 0;
        return true;
    }
    sz total = 0;
    for (sz k = 0; k < list->len; k++) {
        if (BUF != list->ptr[k]->ty) return false;
        if (k) total+= sep->len;
        total+= list->ptr[k]->as.buf.len;
    }
    u8* ptr = malloc(total);
    if (!ptr) return false;
    sz offset = 0;
    for (sz k = 0; k < list->len; k++) {
        if (k) {
            memcpy(ptr+offset, sep->ptr, sep->len);
            offset+= sep->len;
        }
        memcpy(ptr+offset, list->ptr[k]->as.buf.ptr, list->ptr[k]->as.buf.len);
        offset+= list->ptr[k]->as.buf.len;
    }
    self->ptr = ptr;
    self->len = total;
    return true;
}
bool _Join1(Buf* self, Lst const* const list) {
    return _Join2(self, list, &(Buf){0});
}

ctor_simple(2, Len
        , "length of a buffer or list"
        , (1, NUM, _LenB, BUF, from)
        , (1, NUM, _LenL, LST, from)
        );
bool _LenB(Num* self, Buf const* const from) {
    self->val = from->len;
    return true;
}
bool _LenL(Num* self, Lst const* const from) {
    self->val = from->len;
    return true;
}

ctor_simple(3, Range
        , "return a list of integral numbers, start inclusive, end exclusive, default step is 1"
        , (3, LST, _Range3, NUM, start, NUM, end, NUM, step)
        , (2, LST, _Range2, NUM, start, NUM, end)
        , (1, LST, _Range1, NUM, end)
        );
bool _Range3(Lst* self, Num const* const start, Num const* const end, Num const* const step) {
    if (destroyed(self)) {
        free(self->ptr[0]);
        free(self->ptr);
        self->ptr = NULL;
        self->len = 0;
        return true;
    }
    self->len = (end->val - start->val) / step->val;
    Obj* arr = calloc(self->len, sizeof(Obj));
    self->ptr = calloc(self->len, sizeof(Obj*));
    if (!arr || !self->ptr) return false;
    for (sz k = 0, n = start->val; k < self->len; n+= step->val) {
        arr[k].ty = NUM;
        arr[k].as.num.val = n;
        self->ptr[k] = arr+k;
        k++;
    }
    return true;
}
bool _Range2(Lst* self, Num const* const start, Num const* const end) {
    return _Range3(self, start, end, &(Num){.val= 1});
}
bool _Range1(Lst* self, Num const* const end) {
    return _Range2(self, &(Num){0}, end);
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
