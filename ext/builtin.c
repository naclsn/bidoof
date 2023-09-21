#include "../helper.h"

export_names
    ( "At"
    , "Bind"
    , "Call"
    , "Count"
    //, "Decode"
    , "Delim"
    //, "Encode"
    //, "Fold"
    , "Id"
    , "Join"
    , "Len"
    , "Map"
    , "Range"
    , "Read"
    , "Rect"
    , "Repeat"
    , "Reverse"
    , "Slice"
    , "Split"
    , "Write"
    //, "Zip"
    );

ctor_simple(1, At
        , "get item at index in list; index can be negative"
        , (2, ANY, _At, LST, list, NUM, index)
        );
bool _At(Obj* self, Lst const* const list, Num const* const index) {
    if (!self->update) return true;
    sz pindex;
    if (index->val < 0) {
        if (list->len < (sz)-index->val) failf(64, "index (%zu) outside list (%zu)", index->val, list->len);
        pindex = list->len + index->val;
    } else {
        if (list->len <= (sz)index->val) failf(64, "index (%zu) outside list (%zu)", index->val, list->len);
        pindex = index->val;
    }
    self->ty = list->ptr[pindex]->ty;
    self->as = list->ptr[pindex]->as;
    return true;
}

ctor_simple(1, Bind
        , "bind args to function, use :0 :1 and such for the arguments"
        , (2, FUN, _Bind, FUN, fn, LST, args)
        );
bool _Bind_call(Obj* self, Obj* res) {
    (void)res;
    Fun const* const fn = &self->argv[0]->as.fun;
    Lst const* const args = &self->argv[1]->as.lst;
    printf("fn: %p\n", fn->call);
    printf("args: "); obj_show(frommember(args, Obj, as), 0); putchar('\n');
    printf("argc: %u\n", res->argc);
    for (u8 k = 0; k < res->argc; k++) {
        printf("[%u]: ", k);
        obj_show(res->argv[k], 0);
        putchar('\n');
    }
    notify("NIY: bound function");
    return false;
}
bool _Bind(Fun* self, Fun const* const fn, Lst const* const args) {
    (void)fn;
    (void)args;
    self->call = _Bind_call;
    printf("fn: %p\n", fn->call);
    printf("args: "); obj_show(frommember(args, Obj, as), 0); putchar('\n');
    return true;
}

ctor_simple(1, Call
        , "call a function with the given args"
        , (2, ANY, _Call, FUN, fn, LST, args)
        );
bool _Call(Obj* self, Fun const* const fn, Lst const* const args) {
    if (!self->update) {
        if (self->data) {
            Obj* res = self->data;
            bool (*up)(Obj*) = res->update;
            if (up) { res->update = NULL; up(res); }
            free(self->data);
            self->data = NULL;
        }
        return true;
    }
    if (self->data) {
        Obj* res = self->data;
        bool changed = args->len != res->argc;
        for (u8 k = 0; !changed && k < res->argc; k++)
            changed = args->ptr[k] != res->argv[k];
        if (!changed) {
            if (res->update && !res->update(res)) fail("..");
            self->ty = res->ty;
            self->as = res->as;
            return true;
        }
        bool (*up)(Obj*) = res->update;
        if (up) { res->update = NULL; up(res); }
        free(self->data);
        self->data = NULL;
    }
    Obj* fnf = frommember(fn, Obj, as);
    Obj* res = calloc(1, sizeof(Obj) + args->len*sizeof(Obj*));
    res->argc = args->len;
    memcpy(&res->argv, args->ptr, args->len*sizeof(Obj*));
    if (!fn->call(fnf, res)) return free(res), false;
    if (res->update && !res->update(res)) {
        bool (*up)(Obj*) = res->update;
        if (up) { res->update = NULL; up(res); }
        return free(res), false;
    }
    self->data = res;
    self->ty = res->ty;
    self->as = res->as;
    return true;
}

ctor_simple(2, Count
        , "count elements of a list or bytes of a buffer that verify a predicate"
        , (2, NUM, _CountB, BUF, from, FUN, pred)
        , (2, NUM, _CountL, LST, from, FUN, pred)
        );
bool _CountB(Num* self, Buf const* const from, Fun const* const pred) {
    if (destroyed(self)) return true;
    Obj* predf = frommember(pred, Obj, as);
    for (sz k = 0; k < from->len; k++) {
        Obj num = {.ty= NUM, .as.num.val= from->ptr[k]};
        Obj* pnum = &num;
        inline_call_assign(res, predf, 1, &pnum);
            if (NUM != res->ty) fail("predicat result should be a number");
            if (0 != res->as.num.val) self->val++;
        inline_call_failed(res);
            // TODO
        inline_call_cleanup(res);
    }
    return true;
}
bool _CountL(Num* self, Lst const* const from, Fun const* const pred) {
    if (destroyed(self)) return true;
    Obj* predf = frommember(pred, Obj, as);
    for (sz k = 0; k < from->len; k++) {
        inline_call_assign(res, predf, 1, &from->ptr[k]);
            if (NUM != res->ty) fail("predicat result should be a number");
            if (0 != res->as.num.val) self->val++;
        inline_call_failed(res);
            // TODO
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
    if (destroyed(self)) return true;
    sz k;
    for (k = 0; k < under->len - delim->len+1; k++) {
        if (0 == memcmp(under->ptr+k, delim->ptr, delim->len)) goto found;
    }
    k = under->len;
found:
    self->ptr = under->ptr;
    self->len = k;
    return true;
}
bool _Delim1(Buf* self, Buf const* const under) {
    if (destroyed(self)) return true;
    self->ptr = under->ptr;
    for (self->len = 0; self->len < under->len && under->ptr[self->len]; self->len++);
    return true;
}

ctor_simple(1, Id
        , "the identity function"
        , (1, ANY, _Id, ANY, some)
        );
bool _Id(Obj* self, Obj const* const some) {
    if (!self->update) return true;
    self->ty = some->ty;
    self->as = some->as;
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
        return true;
    }
    sz total = 0;
    for (sz k = 0; k < list->len; k++) {
        if (BUF != list->ptr[k]->ty) failf(44, "item at %zu isn't a buffer", k);
        if (k) total+= sep->len;
        total+= list->ptr[k]->as.buf.len;
    }
    u8* ptr = realloc(self->ptr, total);
    if (!ptr) fail("OOM");
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

ctor_simple(1, Map
        , "create a new list with the result of applying the operation to each item of the input list"
        , (2, LST, _Map, FUN, op, LST, input)
        );
bool _Map(Lst* self, Fun const* const op, Lst const* const input) {
    if (destroyed(self)) {
        if (!self->ptr) return true;
        free(*self->ptr);
        free(self->ptr);
        return true;
    }
    sz len = input->len;
    Obj* arr = realloc(self->ptr ? *self->ptr : NULL, len * (sizeof(Obj) + 1*sizeof(Obj*)));
    Obj** ptr = realloc(self->ptr, len * sizeof(Obj*));
    if (!arr || !ptr) {
        free(arr);
        free(ptr);
        fail("OOM");
    }
    Obj* opf = frommember(op, Obj, as);
    for (sz k = 0; k < len; k++) {
        memset(ptr[k] = arr+k, 0, sizeof(Obj));
        ptr[k]->argc = 1;
        ptr[k]->argv[0] = input->ptr[k];
        if (!obj_call(opf, ptr[k])) fail(".."); // XXX(cleanup): all obj_destroy and free
    }
    self->ptr = ptr;
    self->len = len;
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
        if (!self->ptr) return true;
        free(*self->ptr);
        free(self->ptr);
        return true;
    }
    sz len = (end->val - start->val) / step->val;
    Obj* arr = realloc(self->ptr ? *self->ptr : NULL, len * sizeof(Obj));
    Obj** ptr = realloc(self->ptr, len * sizeof(Obj*));
    if (!arr || !ptr) {
        free(arr);
        free(ptr);
        fail("OOM");
    }
    for (sz k = 0, n = start->val; k < len; n+= step->val, k++) {
        memset(arr+k, 0, sizeof(Obj));
        arr[k].ty = NUM;
        arr[k].as.num.val = n;
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
        return true;
    }
    char filez[256] = {0};
    memcpy(filez, file->ptr, file->len < 255 ? file->len : 255);
    FILE *f = fopen(filez, "rb");
    if (!f) failf(32+strlen(filez), "cannot open file '%s' for reading", filez);
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
        if (!self->ptr) return true;
        free(*self->ptr);
        free(self->ptr);
        return true;
    }
    sz w = item_len->val + item_pad->val;
    sz len = under->len / w;
    Obj* arr = realloc(self->ptr ? *self->ptr : NULL, len * sizeof(Obj));
    Obj** ptr = realloc(self->ptr, len * sizeof(Obj*));
    if (!arr || !ptr) {
        free(arr);
        free(ptr);
        fail("OOM");
    }
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

ctor_simple(1, Repeat
        , "repeat its argument"
        , (2, LST, _Repeat, ANY, one, NUM, count)
        );
bool _Repeat(Lst* self, Obj const* const one, Num const* const count) {
    if (destroyed(self)) {
        free(self->ptr);
        return true;
    }
    if (count->val < 0) fail("negative repeat count");
    self->ptr = realloc(self->ptr, count->val * sizeof(Obj*));
    if (!self->ptr) fail("OOM");
    for (sz k = 0; k < (sz)count->val; k++) self->ptr[k] = (Obj*)one;
    self->len = count->val;
    return true;
}

ctor_simple(2, Reverse
        , "reverse a list or a buffer"
        , (1, BUF, _ReverseB, BUF, under)
        , (1, LST, _ReverseL, LST, under)
        );
bool _ReverseB(Buf* self, Buf const* const under) {
    if (destroyed(self)) {
        free(self->ptr);
        return true;
    }
    u8* ptr = realloc(self->ptr, under->len);
    if (!ptr) fail("OOM");
    for (sz k = 0; k < under->len; k++) self->ptr[k] = under->ptr[under->len-1 - k];
    self->ptr = ptr;
    self->len = under->len;
    return true;
}
bool _ReverseL(Lst* self, Lst const* const under) {
    (void)under;
    // TODO
    if (destroyed(self)) return true;
    fail("NIY: Lst Reverse(Lst)");
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
        if (under->len < (sz)-begin->val) failf(64, "slice begin (%zu) outside buffer (%zu)", begin->val, under->len);
        pbegin = under->len + begin->val;
    } else {
        if (under->len <= (sz)begin->val) failf(64, "slice begin (%zu) outside buffer (%zu)", begin->val, under->len);
        pbegin = begin->val;
    }
    if (end->val < 0) {
        if (under->len < (sz)-end->val) failf(64, "slice end (%zu) outside buffer (%zu)", end->val, under->len);
        pend = under->len + end->val;
    } else {
        if (under->len <= (sz)end->val) failf(64, "slice end (%zu) outside buffer (%zu)", end->val, under->len);
        pend = end->val;
    }
    if (pend < pbegin) failf(75, "slice length would be negative [%zu:%zu]", pbegin, pend);
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
        , (2, LST, _Split2, BUF, buffer, BUF, sep)
        , (1, LST, _Split1, BUF, buffer)
        );
bool _Split2(Lst* self, Buf const* const buffer, Buf const* const sep) {
    if (destroyed(self)) {
        if (!self->ptr) return true;
        free(*self->ptr);
        free(self->ptr);
        return true;
    }
    sz reserved = 64;
    sz* founds = malloc(reserved);
    if (!founds) fail("OOM");
    sz len = 0;
    for (sz k = 0; k < buffer->len - sep->len+1; k++) {
        if (0 == memcmp(buffer->ptr+k, sep->ptr, sep->len)) {
            if (reserved-1 == len) {
                sz* niw = malloc(reserved);
                if (!niw) { free(founds); fail("OOM"); }
                founds = niw;
            }
            founds[len++] = k;
            k+= sep->len-1;
        }
    }
    len++;
    Obj* arr = realloc(self->ptr ? *self->ptr : NULL, len * sizeof(Obj));
    Obj** ptr = realloc(self->ptr, len * sizeof(Obj*));
    if (!arr || !ptr) {
        free(founds);
        free(arr);
        free(ptr);
        fail("OOM");
    }
    sz p = 0;
    for (sz k = 0; k < len-1; p = founds[k++] + sep->len) {
        memset(arr+k, 0, sizeof(Obj));
        arr[k].ty = BUF;
        arr[k].as.buf.ptr = buffer->ptr + p;
        arr[k].as.buf.len = founds[k] - p;
        ptr[k] = arr+k;
    }
    memset(arr+len-1, 0, sizeof(Obj));
    arr[len-1].ty = BUF;
    arr[len-1].as.buf.ptr = buffer->ptr + p;
    arr[len-1].as.buf.len = buffer->len - p;
    ptr[len-1] = arr+len-1;
    free(founds);
    self->ptr = ptr;
    self->len = len;
    return true;
}
bool _Split1(Lst* self, Buf const* const buffer) {
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
    if (!f) failf(32+strlen(filez), "cannot open file '%s' for writing", filez);
    fwrite(self->ptr = content->ptr, self->len = content->len, 1, f);
    fclose(f);
    return true;
}
