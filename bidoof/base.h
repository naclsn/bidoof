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
#define track(__vname, __free) __vname = {0}; trackadd(&__vname, (void(*)(void cref))__free); __vname
#define atrack(__tname, __vname) __tname track(__vname, __tname##_free)

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
#define exitf(...) (printf(HERE ": " __VA_ARGS__), putchar(10), exit(1))

#define adapt_bipa_type(__tname)                            \
    typedef struct __tname __tname;                         \
    static inline void __tname##_dump(__tname cref self) {  \
        bipa_dump_##__tname(self, stdout, 0);               \
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

static void xxd(buf const b) {
    for (sz k = 0; k < b.len; k++) {
        if (k && !(k & 0xf)) printf("\n");
        printf("%02X ", b.ptr[k]);
    }
}

#define bfmt(__buf) (int)(__buf)->len, (__buf)->ptr

static void buf_free(buf cref self) {
    free(self->ptr);
}

static void bcat(buf ref to, buf cref other) {
    u8* dest = dyarr_insert(to, to->len, other->len);
    if (!dest) exitf("OOM");
    memcpy(dest, other->ptr, other->len);
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
    FILE* f = fopen((char*)path->ptr, "rb");
    if (!f) exitf("could not open file %.*s", bfmt(path));
    if (0 != fseek(f, 0, SEEK_END)) exitf("could not read file %.*s", bfmt(path));
    r.len = ftell(f);
    r.ptr = malloc(r.len);
    if (!r.ptr) exitf("OOM");
    fread(r.ptr, 1, r.len, f);
    fclose(f);
    return r;
}

static void file_write(buf cref b, buf cref path) {
    FILE* f = fopen((char*)path->ptr, "wb");
    if (!f) exitf("could not open file %.*s", bfmt(path));
    fwrite(b->ptr, 1, b->len, f);
    fclose(f);
}

#endif // __BIDOOF_H__
