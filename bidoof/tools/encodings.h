#include "../base.h"

buf b64_decode(buf cref source);
buf b64_encode(buf cref source);
typedef dyarr(u32) codepoints;
codepoints utf8_decode(buf cref source);
buf utf8_encode(codepoints cref source);

buf b64_decode(buf cref source) _bdf_impl({
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

    buf r = {0};

    if (0 == source->len)
        exitf("source is empty");
    if (0 != source->len%4)
        exitf("length (%zu) is not a multiple of 4", source->len);
    for (sz k = 0; k < source->len; k++)
        if (66 == from64[source->ptr[k]])
            exitf("invalid character 0x%02X at index %zu (/%zu)", source->ptr[k], k, source->len);
    if (2 < source->len && '=' == source->ptr[source->len-3])
        exitf("invalid padding");

    u8 const pad
        = '=' == source->ptr[source->len-2] ? 2
        : '=' == source->ptr[source->len-1] ? 1
        : 0;
    r.len = source->len/4*3 - pad;
    if (!dyarr_resize(&r, r.len)) exitf("OOM");

    sz i = 0, j = 0;
    if (1 < r.len) for (; i < r.len-2; i+= 3, j+= 4) {
        u32 const n
            = (from64[source->ptr[j+0]] << (6*3))
            | (from64[source->ptr[j+1]] << (6*2))
            | (from64[source->ptr[j+2]] << (6*1))
            | (from64[source->ptr[j+3]] << (6*0))
            ;

        r.ptr[i+0] = (n >> (8*2))&0xff;
        r.ptr[i+1] = (n >> (8*1))&0xff;
        r.ptr[i+2] = (n >> (8*0))&0xff;
    }

    if (0 != pad) {
        u32 const n = 1 == pad
            ? (from64[source->ptr[j+0]] << (6*3))
            | (from64[source->ptr[j+1]] << (6*2))
            | (from64[source->ptr[j+2]] << (6*1))
            | 0
            : (from64[source->ptr[j+0]] << (6*3))
            | (from64[source->ptr[j+1]] << (6*2))
            | 0
            | 0
            ;

        r.ptr[i+0] = (n >> (8*2))&0xff;
        if (1 == pad) r.ptr[i+1] = (n >> (8*1))&0xff;
    }

    return r;
})

buf b64_encode(buf cref source) _bdf_impl({
    static u8 const to64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    buf r = {0};

    u8 const mod = source->len%3;
    u8 const pad = 0 != mod ? 3-mod : 0;

    r.len = (source->len + pad) * 4/3;
    if (!dyarr_resize(&r, r.len)) exitf("OOM");

    sz i = 0, j = 0;
    if (1 < source->len) for (; i < source->len-2; i+= 3, j+= 4) {
        unsigned const n
            = (source->ptr[i+0] << (8*2))
            | (source->ptr[i+1] << (8*1))
            | (source->ptr[i+2] << (8*0))
            ;

        r.ptr[j+0] = to64[(n >> (6*3)) & 63];
        r.ptr[j+1] = to64[(n >> (6*2)) & 63];
        r.ptr[j+2] = to64[(n >> (6*1)) & 63];
        r.ptr[j+3] = to64[(n >> (6*0)) & 63];
    }

    if (0 != pad) {
        unsigned const n = 1 == pad
            ? (source->ptr[i+0] << (8*2))
            | (source->ptr[i+1] << (8*1))
            | 0
            : (source->ptr[i+0] << (8*2))
            | 0
            | 0
            ;

        r.ptr[j+0] = to64[(n >> (6*3)) & 63];
        r.ptr[j+1] = to64[(n >> (6*2)) & 63];
        r.ptr[j+2] = to64[(n >> (6*1)) & 63];
        r.ptr[j+3] = to64[(n >> (6*0)) & 63];

        r.ptr[r.len-1] = '=';
        if (2 == pad) r.ptr[r.len-2] = '=';
    }

    return r;
})

codepoints utf8_decode(buf cref source) _bdf_impl({
    codepoints r = {0};
    if (!dyarr_resize(&r, source->len/2)) exitf("OOM");

    for (sz k = 0; k < source->len; k++) {
        u32* const u = dyarr_push(&r);
        if (!u) {
            free(r.ptr);
            exitf("OOM");
        }
        *u = source->ptr[k];

        if (0 == (128 & *u))
            ;
        else if (0 == (32 & *u) && k+1 < source->len) {
            u8 x = source->ptr[++k];
            *u = ((*u & 31) << 6) | (x & 63);
        }
        else if (0 == (16 & *u) && k+2 < source->len) {
            u8 x = source->ptr[++k], y = source->ptr[++k];
            *u = ((*u & 15) << 12) | ((x & 63) << 6) | (y & 63);
        }
        else if (0 == (8 & *u) && k+3 < source->len) {
            u8 x = source->ptr[++k], y = source->ptr[++k], z = source->ptr[++k];
            *u = ((*u & 7) << 18) | ((x & 63) << 12) | ((y & 63) << 6) | (z & 63);
        }
        else {
            free(r.ptr);
            exitf("unexpected byte 0x%02X or end of stream at index %zu (/%zu)", *u, k, source->len);
        }
    }

    return r;
})


#define _push(__val) do {         \
        u8* at = dyarr_push(&r);  \
        if (!at) {                \
            free(r.ptr);          \
            exitf("OOM");         \
        }                         \
        *at = __val;              \
    } while (false)
buf utf8_encode(codepoints cref source) _bdf_impl({
    buf r = {0};
    if (!dyarr_resize(&r, source->len)) exitf("OOM");

    for (sz k = 0; k < source->len; k++) {
        u32 val = source->ptr[k];

        if (val < 128) _push(val);
        else {
            u8 x = val & 63;
            val>>= 6;
            if (val < 32) _push(192 | val);
            else {
                u8 y = val & 63;
                val>>= 6;
                if (val < 16) _push(224 | val);
                else {
                    u8 z = val & 63;
                    _push(240 | (val >> 6));
                    _push(128 | z);
                }
                _push(128 | y);
            }
            _push(128 | x);
        }
    } // for in source

    return r;
})
#undef _push
