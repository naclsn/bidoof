/// Encode and decode semantics are as follow:
/// - SomeEncode: source -> some
/// - SomeDecode: some -> source
///
/// Thus: `SomeDecode (SomeEncode source) == source`;
/// but *not necessarily*: `SomeEncode (SomeDecode source) == source`

#include "../helper.h"

export_names
    ( "Base64Decode"
    , "Base64Encode"
    , "NumDecode"
    , "NumEncode"
    , "Utf8Decode"
    , "Utf8Encode"
    );

ctor_simple(1, Base64Decode
        , "decodes base64 to bytes; does not MIME tho"
        , (1, BUF, _Base64Decode, BUF, source)
        );

static u8 const from64[] = {
    66,66,66,66,66,66,66,66,66,66,64,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,62,66,66,66,63,52,53,
    54,55,56,57,58,59,60,61,66,66,66,65,66,66,66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,66,66,66,66,66,66,26,27,28,
    29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66
};

bool _Base64Decode(Buf* self, Buf const* const source) {
    if (destroyed(self)) return free(self->ptr), true;

    if (0 == source->len)
        fail("source is empty");
    if (0 != source->len%4)
        failf(53, "length (%zu) is not a multiple of 4", source->len);
    for (sz k = 0; k < source->len; k++)
        if (66 == from64[source->ptr[k]])
            failf(49, "invalid character at %zu '%#4X'", k, source->ptr[k]);
    if (2 < source->len && '=' == source->ptr[source->len-3])
        fail("invalid padding");

    u8 pad
        = '=' == source->ptr[source->len-2] ? 2
        : '=' == source->ptr[source->len-1] ? 1
        : 0;
    sz len = source->len/4*3 - pad;
    if (!(self->ptr = malloc(len))) fail("OOM");
    self->len = len;

    sz i = 0, j = 0;
    if (1 < self->len) for (; i < self->len-2; i+= 3, j+= 4) {
        u32 n
            = (from64[source->ptr[j+0]] << (6*3))
            | (from64[source->ptr[j+1]] << (6*2))
            | (from64[source->ptr[j+2]] << (6*1))
            | (from64[source->ptr[j+3]] << (6*0))
            ;

        self->ptr[i+0] = (n >> (8*2))&0xff;
        self->ptr[i+1] = (n >> (8*1))&0xff;
        self->ptr[i+2] = (n >> (8*0))&0xff;
    }

    if (0 != pad) {
        u32 n = 1 == pad
            ? (from64[source->ptr[j+0]] << (6*3))
            | (from64[source->ptr[j+1]] << (6*2))
            | (from64[source->ptr[j+2]] << (6*1))
            | 0
            : (from64[source->ptr[j+0]] << (6*3))
            | (from64[source->ptr[j+1]] << (6*2))
            | 0
            | 0
            ;

        self->ptr[i+0] = (n >> (8*2))&0xff;
        if (1 == pad) self->ptr[i+1] = (n >> (8*1))&0xff;
    }

    return true;
}

ctor_simple(1, Base64Encode
        , "encodes bytes to base64; does not MIME tho"
        , (1, BUF, _Base64Encode, BUF, source)
        );

static u8 const to64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

bool _Base64Encode(Buf* self, Buf const* const source) {
    if (destroyed(self)) return free(self->ptr), true;

    u8 mod = source->len%3;
    u8 pad = 0 != mod ? 3-mod : 0;

    sz len = (source->len + pad) * 4/3;
    if (!(self->ptr = malloc(len))) fail("OOM");
    self->len = len;

    sz i = 0, j = 0;
    if (1 < source->len) for (; i < source->len-2; i+= 3, j+= 4) {
        int n
            = (source->ptr[i+0] << (8*2))
            | (source->ptr[i+1] << (8*1))
            | (source->ptr[i+2] << (8*0))
            ;

        self->ptr[j+0] = to64[(n >> (6*3)) & 0b111111];
        self->ptr[j+1] = to64[(n >> (6*2)) & 0b111111];
        self->ptr[j+2] = to64[(n >> (6*1)) & 0b111111];
        self->ptr[j+3] = to64[(n >> (6*0)) & 0b111111];
    }

    if (0 != pad) {
        int n = 1 == pad
            ? (source->ptr[i+0] << (8*2))
            | (source->ptr[i+1] << (8*1))
            | 0
            : (source->ptr[i+0] << (8*2))
            | 0
            | 0
            ;

        self->ptr[j+0] = to64[(n >> (6*3)) & 0b111111];
        self->ptr[j+1] = to64[(n >> (6*2)) & 0b111111];
        self->ptr[j+2] = to64[(n >> (6*1)) & 0b111111];
        self->ptr[j+3] = to64[(n >> (6*0)) & 0b111111];

        self->ptr[self->len-1] = '=';
        if (2 == pad) self->ptr[self->len-2] = '=';
    }

    return true;
}

ctor_simple(1, NumDecode
        , "decodes bytes to integral value"
        , (3, NUM, _NumDecode, BUF, source, SYM, endian, SYM, sign)
        );

bool _NumDecode(Num* self, Buf const* const source, Sym const* const endian, Sym const* const sign) {
    if (destroyed(self)) return true;

    try_enum_symcvt(Endian, ed, 2, *endian, BIG_END, LITTLE_END);
    try_enum_symcvt(Sign, sg, 2, *sign, SIGNED, UNSIGNED);

    if (8 < source->len) fail("NIY: ints larger than 64 bits");
    if (0 == source->len) return self->val = 0, true;

    i64 val = 0;
    bool ngt = false;

    if (LITTLE_END == ed) {
        ngt = source->ptr[source->len-1] & 0x80;
        for (sz k = 0; k < source->len; k++)
            val = val << 8 | source->ptr[source->len-1-k];
    }

    else {
        ngt = source->ptr[0] & 0x80;
        for (sz k = 0; k < source->len; k++)
            val = val << 8 | source->ptr[k];
    }

    // YYY: meh (sign extends to 64 bits)
    if (SIGNED == sg && ngt) for (sz k = source->len; k < 8; k++) val|= (u64)0xff << k*8;

    self->val = val;
    return true;
}

ctor_simple(1, NumEncode
        , "encode an integral value to bytes"
        , (3, BUF, _NumEncode, NUM, source, SYM, endian, NUM, width)
        );

bool _NumEncode(Buf* self, Num const* const source, Sym const* const endian, Num const* const width) {
    free(self->ptr);
    if (destroyed(self)) return true;

    try_enum_symcvt(Endian, ed, 2, *endian, BIG_END, LITTLE_END);

    if (width->val < 0) fail("width is negative");
    if (8 < width->val) fail("NIY: ints larger than 64 bits");
    if (0 == width->val) return self->len = 0, true;

    self->ptr = malloc(self->len = width->val);
    if (!self->ptr) fail("OOM");

    i64 val = source->val;

    if (LITTLE_END == ed) {
        for (sz k = 0; k < self->len; k++) {
            self->ptr[k] = val & 0xff;
            val<<= 8;
        }
    }

    else {
        for (sz k = 0; k < self->len; k++) {
            self->ptr[self->len-1-k] = val & 0xff;
            val<<= 8;
        }
    }

    return true;
}

ctor_simple(1, Utf8Decode
        , "decodes UTF-8 bytes to unicode codepoints"
        , (1, LST, _Utf8Decode, BUF, source)
        );

bool _Utf8Decode(Lst* self, Buf const* const source) {
    if (self->ptr) free(*self->ptr);
    free(self->ptr);
    if (destroyed(self)) return true;
    self->ptr = NULL;
    self->len = 0;

    dyarr(Obj) arr = {0};
    if (!dyarr_resize(&arr, source->len/2)) fail("OOM");

    for (sz k = 0; k < source->len; k++) {
        u32 u = source->ptr[k];

        if (0 == (0b10000000 & u))
            ;
        else if (0 == (0b00100000 & u) && k+1 < source->len) {
            u8 x = source->ptr[++k];
            u = ((u & 0b00011111) << 6) | (x & 0b00111111);
        }
        else if (0 == (0b00010000 & u) && k+2 < source->len) {
            u8 x = source->ptr[++k], y = source->ptr[++k];
            u = ((u & 0b00001111) << 12) | ((x & 0b00111111) << 6) | (y & 0b00111111);
        }
        else if (0 == (0b00001000 & u) && k+3 < source->len) {
            u8 x = source->ptr[++k], y = source->ptr[++k], z = source->ptr[++k];
            u = ((u & 0b00000111) << 18) | ((x & 0b00111111) << 12) | ((y & 0b00111111) << 6) | (z & 0b00111111);
        }
        else failf(92, "unexpected byte 0x%02X or end of stream at index %zu (/%zu)", u, k, source->len);

        Obj* niw = dyarr_push(&arr);
        if (!niw) fail("OOM");
        memset(niw, 0, sizeof *niw);
        niw->ty = NUM;
        niw->as.num.val = u;
    }

    Obj** ptr = calloc(arr.len, sizeof(Obj**));
    if (!ptr) { free(arr.ptr); fail("OOM"); }
    for (sz k = 0; k < arr.len; k++) ptr[k] = arr.ptr+k;

    self->ptr = ptr;
    self->len = arr.len;
    return true;
}


ctor_simple(1, Utf8Encode
        , "encodes unicode codepoints to UTF-8 bytes"
        , (1, BUF, _Utf8Encode, LST, source)
        );

bool _Utf8Encode(Buf* self, Lst const* const source) {
    free(self->ptr);
    if (destroyed(self)) return true;
    self->ptr = NULL;

    dyarr(u8) arr = {0};
    if (!dyarr_resize(&arr, source->len)) fail("OOM");

    for (sz k = 0; k < source->len; k++) {
        if (NUM != source->ptr[k]->ty) {
            free(self->ptr);
            failf(45, "not a number at index %zu", k);
        }
        u32 val = source->ptr[k]->as.num.val;

#define _push(__val) do {               \
            u8* at = dyarr_push(&arr);  \
            if (!at) fail("OOM");       \
            *at = __val;                \
        } while (false)

        if (val < 0b10000000) _push(val);
        else {
            u8 x = val & 0b00111111;
            val>>= 6;
            if (val < 0b00100000) _push(0b11000000 | val);
            else {
                u8 y = val & 0b00111111;
                val>>= 6;
                if (val < 0b00010000) _push(0b11100000 | val);
                else {
                    u8 z = val & 0b00111111;
                    _push(0b11110000 | (val >> 6));
                    _push(0b10000000 | z);
                }
                _push(0b10000000 | y);
            }
            _push(0b10000000 | x);
        }

#undef _push
    }

    self->ptr = arr.ptr;
    self->len = arr.len;
    return true;
}
