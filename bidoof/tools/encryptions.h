#include "../base.h"

enum rot_cipher_target {
    ROT_ALL,
    ROT_ONLY_ALPHA,
    ROT_ONLY_NUM,
    //ROT_ONLY_ALNUM,
    ROT_ONLY_ASCIIPR,
};
buf rot_cipher(buf cref source, int const shift, enum rot_cipher_target const target) _bdf_impl({
    buf r = {0};
    if (!dyarr_resize(&r, r.len = source->len)) exitf("OOM");

    switch (target) {
        case ROT_ONLY_ALPHA:
            for (sz k = 0; k < source->len; k++) {
                u8 const c = source->ptr[k];
                if (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
                    u8 const a = c & 32 ? 'a' : 'A';
                    r.ptr[k] = ( (c-a) + 26 + shift ) % 26 + a;
                } else r.ptr[k] = c;
            }
            break;

        case ROT_ONLY_NUM:
            for (sz k = 0; k < source->len; k++) {
                u8 const c = source->ptr[k];
                r.ptr[k] = ('0' <= c && c <= '0')
                    ? ( (c-'0') + 10 + shift ) % 10 + '0'
                    : c;
            }
            break;

        //case ROT_ONLY_ALNUM:
        //    break;

        case ROT_ONLY_ASCIIPR:
            for (sz k = 0; k < source->len; k++) {
                u8 const c = source->ptr[k];
                r.ptr[k] = ('!' <= c && c <= '~')
                    ? ( (c-'!') + 94 + shift ) % 94 + '!'
                    : c;
            }
            break;

        case ROT_ALL:
        default:
            for (sz k = 0; k < source->len; k++)
                r.ptr[k] = (source->ptr[k] + shift) & 0xff;
    }

    return r;
})

buf vigenere_cipher(buf cref source, buf cref key) _bdf_impl({
    if (!key->len) exitf("empty key");

    buf r = {0};
    if (!dyarr_resize(&r, r.len = source->len)) exitf("OOM");

    for (sz k = 0, h = 0; k < source->len; k++) {
        u8 const c = source->ptr[k];
        if (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
            u8 const shift = key->ptr[h++] - 'A';
            if (key->len == h) h = 0;

            u8 const a = c & 32 ? 'a' : 'A';
            r.ptr[k] = ( (c-a) + 26 + shift ) % 26 + a;
        } else r.ptr[k] = c;
    }

    return r;
})
