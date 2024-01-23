#include "../base.h"

u32 crc32(buf cref s);

#ifdef BDF_IMPLEMENTATION

u32 crc32(buf cref s) {
    u32 crc = 0xffffffff;
    for (sz i = 0; i < s->len; i++) {
        crc^= s->ptr[i];
        for (unsigned j = 0; j < 8; j++)
            crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
    }
    return ~crc;
}

#endif // BDF_IMPLEMENTATION
