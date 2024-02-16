#ifndef __BIDOOF_T_CHECKS__
#define __BIDOOF_T_CHECKS__

#include "../base.h"

#ifdef BIDOOF_LIST_DEPS
static struct _list_deps_item const _list_deps_me_checks = {_list_deps_first, "checks"};
#undef _list_deps_first
#define _list_deps_first &_list_deps_me_checks
#endif

u32 crc32(buf const s);
u32 adler32(buf const s);

#ifdef BIDOOF_IMPLEMENTATION

u32 crc32(buf const s) {
    u32 crc = 0xffffffff;
    for (sz i = 0; i < s.len; i++) {
        crc^= s.ptr[i];
        for (unsigned j = 0; j < 8; j++)
            crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
    }
    return ~crc;
}

u32 adler32(buf const s) {
    u32 adler = 1;
    u32 s1 = adler & 0xffff;
    u32 s2 = (adler >> 16) & 0xffff;
    for (sz k = 0; k < s.len; k++) {
        s1 = (s1 + s.ptr[k]) % 65521;
        s2 = (s2 + s1)        % 65521;
    }
    return (s2 << 16) + s1;
}

#endif // BIDOOF_IMPLEMENTATION

#endif // __BIDOOF_T_CHECKS__
