#include "../helper.h"

export_names("ViewTxt");

ctor_w_also(1, ViewTxt, _ViewTxt_once
        , "view and edit buffer as text"
        , (1, BUF, _ViewTxt, BUF, txt)
        );

typedef struct ViewTxtState {
    char const* things;
    u8 ptr[];
} ViewTxtState;

bool _ViewTxt_once(Obj* fun, Obj* res) {
    (void)fun;
    puts("== create the thingy");

    ViewTxtState* st = malloc(sizeof *st);
    st->things = "the things";

    res->as.buf.ptr = st->ptr;

    return true;
}

bool _ViewTxt(Obj* self, Buf* r, Buf const* const txt) {
    ViewTxtState* st = frommember(r->ptr, ViewTxtState, ptr);
    printf("== retrieved the thingy: '%s'\n", st->things);

    if (!self->update) {
        puts("== destroy the thingy");
        free(st);
        return true;
    }

    printf("== update the thingy <<%.*s>>\n", (int)txt->len, txt->ptr);

    if (r->len < txt->len) {
        ViewTxtState tmp = *st;

        free(st);
        st = malloc(sizeof *st + txt->len);
        if (!st) return false;
        memcpy(st, &tmp, sizeof *st);

        r->ptr = st->ptr;
    }

    memcpy(r->ptr, txt->ptr, r->len = txt->len);

    return true;
}
