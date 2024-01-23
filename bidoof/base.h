#ifndef __BIDOOF_H__
#define __BIDOOF_H__

///#if 1//def BIDOOF_IMPLEMENTATION
///#define _bidoof_impl(...) __VA_ARGS__
///#else
///#define _bidoof_impl(...) ;
///#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dyarr.h"

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;
typedef float     f32;
typedef double    f64;
typedef size_t    sz;
typedef dyarr(u8) buf;

#define mk(__c) (buf){.ptr= (u8*)__c, .len= strlen(__c)}

#define ref * const
#define cref const ref
#define let(__tname, __vname) __tname __vname = {0}; trackadd(&__vname, (void(*)(void cref))__tname##_free); __vname

struct trackent {
    void const* self;
    void (*free)(void cref);
};
static dyarr(struct trackent) tracked = {0};
static inline void trackadd(void cref self, void (*free)(void cref)) {
    *dyarr_push(&tracked) = (struct trackent){self, free};
}
static void cleanup(void) {
    for (sz k = 0; k < tracked.len; k++)
        tracked.ptr[k].free(tracked.ptr[k].self);
    dyarr_clear(&tracked);
}

#define _HERE_STR(__ln) #__ln
#define _HERE_XSTR(__ln) _HERE_STR(__ln)
#define HERE __FILE__ ":" _HERE_XSTR(__LINE__)
#define exitf(...) (printf(HERE ": " __VA_ARGS__), putchar('\n'), exit(1))

#define swapn(__x, __y, __sz) do {  \
    sz _sz = __sz;                  \
    u8 tmp[__sz];                   \
    memcpy(tmp, &(__y), _sz);       \
    memcpy(&(__y), &(__x),  _sz);   \
    memcpy(&(__x), tmp, _sz);       \
} while (0)
#define swap(__x, __y) swapn((__x), (__y), sizeof(__x))

#define adapt_bipa_type(__tname)                            \
    typedef struct __tname __tname;                         \
    static inline void __tname##_dump(__tname cref self) {  \
        bipa_dump_##__tname(self, stdout, 0);               \
        putchar('\n');                                      \
    }                                                       \
    static inline buf __tname##_build(__tname cref self) {  \
        buf r = {0};                                        \
        if (!bipa_build_##__tname(self, &r))                \
            exitf("could not build a " #__tname);           \
        return r;                                           \
    }                                                       \
    static inline __tname __tname##_parse(buf cref buf) {   \
        struct __tname r;                                   \
        sz at = 0;                                          \
        if (!bipa_parse_##__tname(&r, buf, &at))            \
            exitf("could not parse a " #__tname);           \
        return r;                                           \
    }                                                       \
    static inline void __tname##_free(__tname cref self) {  \
        bipa_free_##__tname(self);                          \
    }

static void xxd(buf cref b) {
    if (0 == b->len) return;
    for (sz j = 0; j < (b->len-1)/16+1; j++) {
        for (sz i = 0; i < 16; i++) {
            sz const k = i+16*j;
            if (b->len < k) printf("   ");
            else printf("%02X ", b->ptr[k]);
        }
        printf("       ");
        for (sz i = 0; i < 16; i++) {
            sz const k = i+16*j;
            if (b->len < k) break;
            char const it = b->ptr[i+16*j];
            printf("%c", ' ' <= it && it <= '~' ? it : '.');
        }
        printf("\n");
    }
}

#define bfmt(__buf) (int)(__buf)->len, (__buf)->ptr

static void buf_free(buf cref self) {
    free(self->ptr);
}

static buf bufcpy(u8 cref ptr, sz const len) {
    buf r = {.ptr= malloc(len), .len= len, .cap= len};
    if (!r.ptr) exitf("OOM");
    memcpy(r.ptr, ptr, len);
    return r;
}

static void bufcat(buf ref to, buf cref other) {
    u8* dest = dyarr_insert(to, to->len, other->len);
    if (!dest) exitf("OOM");
    memcpy(dest, other->ptr, other->len);
}

static u16 peek16le(buf cref b, sz const k) {
    return (u16)b->ptr[k]
         | (u16)b->ptr[k+1]<<8;
}
static u32 peek32le(buf cref b, sz const k) {
    return (u32)b->ptr[k]
         | (u32)b->ptr[k+1]<<8
         | (u32)b->ptr[k+2]<<16
         | (u32)b->ptr[k+3]<<24;
}
static u64 peek64le(buf cref b, sz const k) {
    return (u64)b->ptr[k]
         | (u64)b->ptr[k+1]<<8
         | (u64)b->ptr[k+2]<<16
         | (u64)b->ptr[k+3]<<24
         | (u64)b->ptr[k+4]<<32
         | (u64)b->ptr[k+5]<<40
         | (u64)b->ptr[k+6]<<48
         | (u64)b->ptr[k+7]<<56;
}

static void poke16le(buf ref b, sz const k, u16 const v) {
    b->ptr[k] = v&0xff;
    b->ptr[k+1] = (v>>8)&0xff;
}
static void poke32le(buf ref b, sz const k, u32 const v) {
    b->ptr[k] = v&0xff;
    b->ptr[k+1] = (v>>8)&0xff;
    b->ptr[k+2] = (v>>16)&0xff;
    b->ptr[k+3] = (v>>24)&0xff;
}
static void poke64le(buf ref b, sz const k, u64 const v) {
    b->ptr[k] = v&0xff;
    b->ptr[k+1] = (v>>8)&0xff;
    b->ptr[k+2] = (v>>16)&0xff;
    b->ptr[k+3] = (v>>24)&0xff;
    b->ptr[k+4] = (v>>32)&0xff;
    b->ptr[k+5] = (v>>40)&0xff;
    b->ptr[k+6] = (v>>48)&0xff;
    b->ptr[k+7] = (v>>56)&0xff;
}

static u16 peek16be(buf cref b, sz const k) {
    return (u16)b->ptr[k]<<8
         | (u16)b->ptr[k+1];
}
static u32 peek32be(buf cref b, sz const k) {
    return (u32)b->ptr[k]<<24
         | (u32)b->ptr[k+1]<<16
         | (u32)b->ptr[k+2]<<8
         | (u32)b->ptr[k+3];
}
static u64 peek64be(buf cref b, sz const k) {
    return (u64)b->ptr[k]<<56
         | (u64)b->ptr[k+1]<<48
         | (u64)b->ptr[k+2]<<40
         | (u64)b->ptr[k+3]<<32
         | (u64)b->ptr[k+4]<<24
         | (u64)b->ptr[k+5]<<16
         | (u64)b->ptr[k+6]<<8
         | (u64)b->ptr[k+7];
}

static void poke16be(buf ref b, sz const k, u16 const v) {
    b->ptr[k] = (v>>8)&0xff;
    b->ptr[k+1] = v&0xff;
}
static void poke32be(buf ref b, sz const k, u32 const v) {
    b->ptr[k] = (v>>24)&0xff;
    b->ptr[k+1] = (v>>16)&0xff;
    b->ptr[k+2] = (v>>8)&0xff;
    b->ptr[k+3] = v&0xff;
}
static void poke64be(buf ref b, sz const k, u64 const v) {
    b->ptr[k] = (v>>56)&0xff;
    b->ptr[k+1] = (v>>48)&0xff;
    b->ptr[k+2] = (v>>40)&0xff;
    b->ptr[k+3] = (v>>32)&0xff;
    b->ptr[k+4] = (v>>24)&0xff;
    b->ptr[k+5] = (v>>16)&0xff;
    b->ptr[k+6] = (v>>8)&0xff;
    b->ptr[k+7] = v&0xff;
}

#define with_buf_as_stream(__buf, __name, ...) do {         \
    FILE* __name = tmpfile();                               \
    if (!__name) exitf("could not open a temporary file");  \
    { __VA_ARGS__; }                                        \
    (__buf)->len = ftell(__name);                           \
    (__buf)->ptr = malloc(r.len);                           \
    if (!(__buf)->ptr) exitf("OOM");                        \
    fseek(__name, 0, SEEK_SET);                             \
    fread((__buf)->ptr, 1, (__buf)->len, __name);           \
    fclose(__name);                                         \
} while (0)

static buf file_read(buf cref path) {
    buf r = {0};
    char local[path->len+1];
    memcpy(local, path->ptr, path->len);
    local[path->len] = '\0';
    FILE* f = fopen(local, "rb");
    if (!f) exitf("could not open file %.*s", bfmt(path));
    if (0 != fseek(f, 0, SEEK_END)) exitf("could not read file %.*s", bfmt(path));
    r.len = ftell(f);
    r.ptr = malloc(r.len);
    if (!r.ptr) exitf("OOM");
    fseek(f, 0, SEEK_SET);
    fread(r.ptr, 1, r.len, f);
    fclose(f);
    return r;
}

static void file_write(buf cref b, buf cref path) {
    char local[path->len+1];
    memcpy(local, path->ptr, path->len);
    local[path->len] = '\0';
    FILE* f = fopen(local, "wb");
    if (!f) exitf("could not open file %.*s", bfmt(path));
    fwrite(b->ptr, 1, b->len, f);
    fclose(f);
}

#endif // __BIDOOF_H__
