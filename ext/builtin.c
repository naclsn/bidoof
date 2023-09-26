#include "../helper.h"
#include <math.h>

#define freenul(__v) free(__v), __v = NULL;

export_names
    ( "Abs"
    , "Add"
    , "At"
    , "Bind"
    , "Call"
    , "Count"
    , "Delim"
    , "Div"
    , "Id"
    , "Join"
    , "Len"
    , "Map"
    , "Mul"
    , "Pow"
    , "Range"
    , "Read"
    , "Rect"
    , "Repeat"
    , "Reverse"
    , "Slice"
    , "Split"
    , "Sub"
    , "Write"
    //, "Fold"
    //, "Zip"
    );

ctor_simple(2, Abs, "abs", (1, NUM, _AbsN, NUM, v), (1, FLT, _AbsF, FLT, v));
bool _AbsN(Num* self, Num const* const v) { self->val = labs(v->val); return true; }
bool _AbsF(Flt* self, Flt const* const v) { self->val = fabsl(v->val); return true; }

ctor_simple(2, Add, "add", (2, NUM, _AddN, NUM, l, NUM, r), (2, FLT, _AddF, FLT, l, FLT, r));
bool _AddN(Num* self, Num const* const l, Num const* const r) { self->val = l->val + r->val; return true; }
bool _AddF(Flt* self, Flt const* const l, Flt const* const r) { self->val = l->val + r->val; return true; }

ctor_simple(2, Div, "div", (2, NUM, _DivN, NUM, l, NUM, r), (2, FLT, _DivF, FLT, l, FLT, r));
bool _DivN(Num* self, Num const* const l, Num const* const r) { self->val = l->val / r->val; return true; }
bool _DivF(Flt* self, Flt const* const l, Flt const* const r) { self->val = l->val / r->val; return true; }

ctor_simple(2, Mul, "mul", (2, NUM, _MulN, NUM, l, NUM, r), (2, FLT, _MulF, FLT, l, FLT, r));
bool _MulN(Num* self, Num const* const l, Num const* const r) { self->val = l->val * r->val; return true; }
bool _MulF(Flt* self, Flt const* const l, Flt const* const r) { self->val = l->val * r->val; return true; }

ctor_simple(1, Pow, "pow", (2, FLT, _SubF, FLT, l, FLT, r));
bool _PowF(Flt* self, Flt const* const l, Flt const* const r) { self->val = powl(l->val, r->val); return true; }

ctor_simple(2, Sub, "sub", (2, NUM, _SubN, NUM, l, NUM, r), (2, FLT, _SubF, FLT, l, FLT, r));
bool _SubN(Num* self, Num const* const l, Num const* const r) { self->val = l->val - r->val; return true; }
bool _SubF(Flt* self, Flt const* const l, Flt const* const r) { self->val = l->val - r->val; return true; }

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
            freenul(self->data);
        }
        return true;
    }
    if (self->data) {
        Obj* res = self->data;
        // XXX: yes but also no, to see if something was updated it will
        // probably be indicated by the cycle
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
        freenul(self->data);
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
    // TODO: if the pred didn't change, we only need to check if the items in from did...
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
    freenul(self->ptr);
    if (destroyed(self)) return true;
    self->len = 0;
    for (sz k = 0; k < list->len; k++) {
        if (BUF != list->ptr[k]->ty) failf(44, "item at %zu isn't a buffer", k);
        if (k) self->len+= sep->len;
        self->len+= list->ptr[k]->as.buf.len;
    }
    if (!self->len) return true;
    self->ptr = malloc(self->len);
    if (!self->ptr) fail("OOM");
    sz offset = 0;
    for (sz k = 0; k < list->len; k++) {
        if (k) {
            memcpy(self->ptr+offset, sep->ptr, sep->len);
            offset+= sep->len;
        }
        memcpy(self->ptr+offset, list->ptr[k]->as.buf.ptr, list->ptr[k]->as.buf.len);
        offset+= list->ptr[k]->as.buf.len;
    }
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
    // TODO/XXX/FIXME: many things are not right
    // - the items are not destroyed before the free(*..)
    // - the input list will be marked updated and cause a full update even
    //   when a single item was actually updated; Map should play is smart
    //   enough to not do more that what's needed
    // - because we return a list of proper objects, we probably also need
    //   to be proper in updated ourself
    if (self->ptr) free(*self->ptr);
    freenul(self->ptr);
    if (destroyed(self)) return true;
    self->len = input->len;
    if (!self->len) return true;
    Obj* arr = malloc(self->len * (sizeof(Obj) + 1*sizeof(Obj*)));
    self->ptr = malloc(self->len * sizeof(Obj*));
    if (!arr || !self->ptr) {
        free(arr);
        freenul(self->ptr);
        fail("OOM");
    }
    Obj* opf = frommember(op, Obj, as);
    for (sz k = 0; k < self->len; k++) {
        memset(self->ptr[k] = arr+k, 0, sizeof(Obj));
        self->ptr[k]->argc = 1;
        self->ptr[k]->argv[0] = input->ptr[k];
        if (!obj_call(opf, self->ptr[k])) fail(".."); // XXX(cleanup): all obj_destroy and free
    }
    return true;
}

ctor_simple(3, Range
        , "return a list of integral numbers, start inclusive, end exclusive, default step is 1"
        , (3, LST, _Range3, NUM, start, NUM, end, NUM, step)
        , (2, LST, _Range2, NUM, start, NUM, end)
        , (1, LST, _Range1, NUM, end)
        );
bool _Range3(Lst* self, Num const* const start, Num const* const end, Num const* const step) {
    if (self->ptr) free(*self->ptr);
    freenul(self->ptr);
    if (destroyed(self)) return true;
    self->len = (end->val - start->val) / step->val;
    if (!self->len) return true;
    Obj* arr = malloc(self->len * sizeof(Obj));
    self->ptr = malloc(self->len * sizeof(Obj*));
    if (!arr || !self->ptr) {
        free(arr);
        freenul(self->ptr);
        fail("OOM");
    }
    for (sz k = 0, n = start->val; k < self->len; n+= step->val, k++) {
        arr[k].ty = NUM;
        arr[k].as.num.val = n;
        self->ptr[k] = arr+k;
    }
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
    freenul(self->ptr);
    if (destroyed(self)) return true;
    char filez[256] = {0}; // XXX: file path length limitation (what is that, DOS?)
    memcpy(filez, file->ptr, file->len < 255 ? file->len : 255);
    FILE *f = fopen(filez, "rb");
    if (!f) failf(32+strlen(filez), "cannot open file '%s' for reading", filez);
    fseek(f, 0, SEEK_END);
    self->len = ftell(f);
    if (self->len) {
        fseek(f, 0, SEEK_SET);
        self->ptr = malloc(self->len);
        if (!self->ptr) { fclose(f); fail("OOM"); }
        fread(self->ptr, self->len, 1, f);
    }
    fclose(f);
    return true;
}

ctor_simple(2, Rect
        , "slices at regular interval into a list of same-size buffers with optional padding"
        , (3, LST, _Rect3, BUF, under, NUM, item_len, NUM, item_pad)
        , (2, LST, _Rect2, BUF, under, NUM, item_len)
        );
bool _Rect3(Lst* self, Buf const* const under, Num const* const item_len, Num const* const item_pad) {
    if (self->ptr) free(*self->ptr);
    freenul(self->ptr);
    if (destroyed(self)) return true;
    sz w = item_len->val + item_pad->val;
    self->len = under->len / w;
    if (!self->len) return true;
    Obj* arr = malloc(self->len * sizeof(Obj));
    self->ptr = malloc(self->len * sizeof(Obj*));
    if (!arr || !self->ptr) {
        free(arr);
        freenul(self->ptr);
        fail("OOM");
    }
    for (sz k = 0; k < self->len; k++) {
        arr[k].ty = BUF;
        arr[k].as.buf.ptr = under->ptr + (w*k);
        arr[k].as.buf.len = item_len->val;
        self->ptr[k] = arr+k;
    }
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
    freenul(self->ptr);
    if (destroyed(self)) return true;
    if (count->val < 0) fail("negative repeat count");
    self->len = count->val;
    if (!self->len) return true;
    self->ptr = malloc(self->len * sizeof(Obj*));
    if (!self->ptr) fail("OOM");
    for (sz k = 0; k < (sz)count->val; k++) self->ptr[k] = (Obj*)one;
    return true;
}

ctor_simple(2, Reverse
        , "reverse a list or a buffer"
        , (1, BUF, _ReverseB, BUF, under)
        , (1, LST, _ReverseL, LST, under)
        );
bool _ReverseB(Buf* self, Buf const* const under) {
    freenul(self->ptr);
    if (destroyed(self)) return true;
    self->len = under->len;
    if (!self->len) return true;
    self->ptr = malloc(self->len);
    if (!self->ptr) fail("OOM");
    for (sz k = 0; k < under->len; k++) self->ptr[k] = under->ptr[under->len-1 - k];
    return true;
}
bool _ReverseL(Lst* self, Lst const* const under) {
    freenul(self->ptr);
    if (destroyed(self)) return true;
    self->len = under->len;
    if (!self->len) return true;
    Obj* arr = malloc(self->len * sizeof(Obj));
    self->ptr = malloc(self->len * sizeof(Obj*));
    if (!arr || !self->ptr) {
        free(arr);
        freenul(self->ptr);
        fail("OOM");
    }
    for (sz k = 0; k < under->len; k++) {
        arr[k].ty = under->ptr[under->len-1 - k]->ty;
        arr[k].as = under->ptr[under->len-1 - k]->as;
        self->ptr[k] = arr+k;
    }
    return true;
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
    if (self->ptr) free(*self->ptr);
    freenul(self->ptr);
    if (destroyed(self)) return true;
    dyarr(sz) founds;
    self->len = 0;
    for (sz k = 0; k < buffer->len - sep->len+1; k++) {
        if (0 == memcmp(buffer->ptr+k, sep->ptr, sep->len)) {
            sz* at = dyarr_push(&founds);
            if (!at) {
                free(founds.ptr);
                fail("OOM");
            }
            *at = k;
            k+= sep->len-1;
        }
    }
    self->len = founds.len+1;
    Obj* arr = malloc(self->len * sizeof(Obj));
    self->ptr = malloc(self->len * sizeof(Obj*));
    if (!arr || !self->ptr) {
        free(founds.ptr);
        free(arr);
        freenul(self->ptr);
        fail("OOM");
    }
    sz p = 0;
    for (sz k = 0; k < founds.len; p = founds.ptr[k++] + sep->len) {
        arr[k].ty = BUF;
        arr[k].as.buf.ptr = buffer->ptr + p;
        arr[k].as.buf.len = founds.ptr[k] - p;
        self->ptr[k] = arr+k;
    }
    arr[self->len-1].ty = BUF;
    arr[self->len-1].as.buf.ptr = buffer->ptr + p;
    arr[self->len-1].as.buf.len = buffer->len - p;
    self->ptr[self->len-1] = arr+self->len-1;
    free(founds.ptr);
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
    if (destroyed(self)) return true; // XXX: should it disregard and write out anyways?
    char filez[256] = {0}; // XXX: file path length limitation (what is that, DOS?)
    memcpy(filez, file->ptr, file->len < 255 ? file->len : 255);
    FILE *f = fopen(filez, "wb");
    if (!f) failf(32+strlen(filez), "cannot open file '%s' for writing", filez);
    fwrite(self->ptr = content->ptr, self->len = content->len, 1, f);
    fclose(f);
    return true;
}
