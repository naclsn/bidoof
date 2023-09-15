#include "../helper.h"

export_names
    //( "At"
    //, "Bind"
    ( "Count"
    //, "Decode"
    , "Delim"
    //, "Encode"
    //, "Fold"
    //, "Id"
    , "Join"
    , "Len"
    //, "Map"
    , "Range"
    , "Read"
    , "Rect"
    //, "Repeat"
    , "Reverse"
    , "Slice"
    , "Split"
    , "Write"
    //, "Zip"
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
    u8* ptr = realloc(self->ptr, total);
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
    sz len = (end->val - start->val) / step->val;
    Obj* arr = realloc(self->ptr ? self->ptr[0] : NULL, len * sizeof(Obj));
    Obj** ptr = realloc(self->ptr, len * sizeof(Obj*));
    if (!arr || !ptr) return free(arr), free(ptr), false;
    for (sz k = 0, n = start->val; k < len; n+= step->val, k++) {
        memset(arr+k, 0, sizeof(Obj));
        arr[k].ty = NUM;
        arr[k].as.num.val = n;
        arr[k].keepalive++;
        ptr[k] = arr+k;
    }
    self->ptr = ptr;
    self->len = len;
    return true;
}
bool _Range2(Lst* self, Num const* const start, Num const* const end) {
    return _Range3(self, start, end, &(Num){.val= 1});
}
bool _Range1(Lst* self, Num const* const end) {
    return _Range2(self, &(Num){0}, end);
}

ctor_simple(1, Read
        , "read a file into a buffer"
        , (1, BUF, _Read, BUF, file)
        );
bool _Read(Buf* self, Buf const* const file) {
    if (destroyed(self)) {
        free(self->ptr);
        self->ptr = NULL;
        self->len = 0;
        return true;
    }
    char filez[256] = {0};
    memcpy(filez, file->ptr, file->len < 255 ? file->len : 255);
    FILE *f = fopen(filez, "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    self->len = ftell(f);
    if (!self->len) {
        fclose(f);
        return true;
    }
    fseek(f, 0, SEEK_SET);
    self->ptr = malloc(self->len);
    if (!self->ptr) self->len = 0;
    else fread(self->ptr, self->len, 1, f);
    fclose(f);
    return !!self->ptr;
}

ctor_simple(2, Rect
        , "slices at regular interval into a list of same-size buffers with optional padding"
        , (3, LST, _Rect3, BUF, under, NUM, item_len, NUM, item_pad)
        , (2, LST, _Rect2, BUF, under, NUM, item_len)
        );
bool _Rect3(Lst* self, Buf const* const under, Num const* const item_len, Num const* const item_pad) {
    if (destroyed(self)) {
        free(self->ptr[0]);
        free(self->ptr);
        self->ptr = NULL;
        self->len = 0;
        return true;
    }
    sz w = item_len->val + item_pad->val;
    sz len = under->len / w;
    Obj* arr = realloc(self->ptr ? self->ptr[0] : NULL, len * sizeof(Obj));
    Obj** ptr = realloc(self->ptr, len * sizeof(Obj*));
    if (!arr || !ptr) return free(arr), free(ptr), false;
    for (sz k = 0; k < len; k++) {
        memset(arr+k, 0, sizeof(Obj));
        arr[k].ty = BUF;
        arr[k].as.buf.ptr = under->ptr + (w*k);
        arr[k].as.buf.len = item_len->val;
        ptr[k] = arr+k;
    }
    self->ptr = ptr;
    self->len = len;
    return true;
}
bool _Rect2(Lst* self, Buf const* const under, Num const* const item_len) {
    return _Rect3(self, under, item_len, &(Num){0});
}

//ctor_simple(1, Repeat
//        , "repeat its argument"
//        , (2, LST, _Repeat, ANY, one, NUM, count)
//        );
//bool _Repeat(Lst* self, Obj const* const one, Num const* const count) {
//    return false;
//}

ctor_simple(2, Reverse
        , "reverse a list or a buffer"
        , (1, BUF, _ReverseB, BUF, under)
        , (1, LST, _ReverseL, LST, under)
        );
bool _ReverseB(Buf* self, Buf const* const under) {
    if (destroyed(self)) {
        free(self->ptr);
        self->ptr = NULL;
        self->len = 0;
        return true;
    }
    u8* ptr = realloc(self->ptr, under->len);
    if (!ptr) return false;
    for (sz k = 0; k < under->len; k++) self->ptr[k] = under->ptr[under->len-1 - k];
    self->ptr = ptr;
    self->len = under->len;
    return true;
}
bool _ReverseL(Lst* self, Lst const* const under) {
    (void)self;
    (void)under;
    // TODO
    notify("NIY: Lst Reverse(Lst)");
    return false;
}

ctor_simple(3, Slice
        , "[begin:end], defaults are begin=0 and end=length; either can be negative"
        , (3, BUF, _Slice3, BUF, under, NUM, begin, NUM, end)
        , (2, BUF, _Slice2, BUF, under, NUM, begin)
        , (1, BUF, _Slice1, BUF, under)
        );
bool _Slice3(Buf* self, Buf const* const under, Num const* const begin, Num const* const end) {
    sz pbegin, pend;
    if (begin->val < 0) {
        if (under->len < (sz)-begin->val) return false;
        pbegin = under->len + begin->val;
    } else {
        if (under->len <= (sz)begin->val) return false;
        pbegin = begin->val;
    }
    if (end->val < 0) {
        if (under->len < (sz)-end->val) return false;
        pend = under->len + end->val;
    } else {
        if (under->len <= (sz)end->val) return false;
        pend = end->val;
    }
    if (pend < pbegin) return false;
    self->ptr = under->ptr + pbegin;
    self->len = pend - pbegin;
    return true;
}
bool _Slice2(Buf* self, Buf const* const under, Num const* const begin) {
    return _Slice3(self, under, begin, &(Num){.val= under->len-1});
}
bool _Slice1(Buf* self, Buf const* const under) {
    return _Slice2(self, under, &(Num){0});
}

ctor_simple(2, Split
        , "split on separator (exclusive) - default delimiter is \"\" (empty)"
        , (2, BUF, _Split2, BUF, buffer, BUF, sep)
        , (1, BUF, _Split1, BUF, buffer)
        );
bool _Split2(Buf* self, Buf const* const buffer, Buf const* const sep) {
    (void)self;
    (void)buffer;
    (void)sep;
    // TODO
    notify("NIY: Buf Split(Buf, Buf)");
    return false;
}
bool _Split1(Buf* self, Buf const* const buffer) {
    return _Split2(self, buffer, &(Buf){0});
}

ctor_simple(1, Write
        , "write a buffer into a file (function is transparent)"
        , (2, BUF, _Write, BUF, file, BUF, content)
        );
bool _Write(Buf* self, Buf const* const file, Buf const* const content) {
    char filez[256] = {0};
    memcpy(filez, file->ptr, file->len < 255 ? file->len : 255);
    FILE *f = fopen(filez, "wb");
    if (!f) return false;
    fwrite(self->ptr = content->ptr, self->len = content->len, 1, f);
    fclose(f);
    return true;
}
