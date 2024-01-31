#include "../base.h"

u32 crc32(buf cref s) _bdf_impl({
    u32 crc = 0xffffffff;
    for (sz i = 0; i < s->len; i++) {
        crc^= s->ptr[i];
        for (unsigned j = 0; j < 8; j++)
            crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
    }
    return ~crc;
})

u32 adler32(buf cref s) _bdf_impl({
    u32 adler = 1;
    u32 s1 = adler & 0xffff;
    u32 s2 = (adler >> 16) & 0xffff;
    for (sz k = 0; k < s->len; k++) {
        s1 = (s1 + s->ptr[k]) % 65521;
        s2 = (s2 + s1)        % 65521;
    }
    return (s2 << 16) + s1;
})
