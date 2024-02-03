#ifndef __BIDOOF_H__
#define __BIDOOF_H__

#ifdef BIDOOF_T_IMPLEMENTATION
#undef BIDOOF_IMPLEMENTATION
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/dyarr.h"

#define mkbuf(__c) (buf){.ptr= (u8*)__c, .len= strlen(__c)}
#define mkbufsl(__b, __st, __ed) (buf){.ptr= (__b)->ptr+(__st), .len= (__ed)-(__st)}

#define ref * const
#define cref const ref
#define opref * const
#define opcref const opref
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

void xxd(buf cref b, sz const l);
void xxdiff(buf cref a, buf cref b, sz const l);
char const* binstr(u64 n, unsigned const w);

void buf_free(buf cref self);

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

buf bufcpy(u8 cref ptr, sz const len);
void bufcat(buf ref to, buf cref other);

u16 peek16le(buf cref b, sz const k);
u32 peek32le(buf cref b, sz const k);
u64 peek64le(buf cref b, sz const k);
void poke16le(buf ref b, sz const k, u16 const v);
void poke32le(buf ref b, sz const k, u32 const v);
void poke64le(buf ref b, sz const k, u64 const v);
u16 peek16be(buf cref b, sz const k);
u32 peek32be(buf cref b, sz const k);
u64 peek64be(buf cref b, sz const k);
void poke16be(buf ref b, sz const k, u16 const v);
void poke32be(buf ref b, sz const k, u32 const v);
void poke64be(buf ref b, sz const k, u64 const v);

buf file_read(buf cref path);
void file_write(buf cref path, buf cref b);

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

#ifdef BIDOOF_IMPLEMENTATION

void xxd(buf cref b, sz const ln) {
    if (0 == b->len) return;
    for (sz j = 0; j < ln && j < (b->len-1)/16+1; j++) {
        printf("%07zx0:   ", j);
        for (sz i = 0; i < 16; i++) {
            sz const k = i+16*j;
            if (b->len <= k) printf("   ");
            else printf("%02X ", b->ptr[k]);
        }
        printf("       ");
        for (sz i = 0; i < 16; i++) {
            sz const k = i+16*j;
            if (b->len <= k) break;
            char const it = b->ptr[i+16*j];
            printf("%c", ' ' <= it && it <= '~' ? it : '.');
        }
        printf("\n");
    }
}

void xxdiff(buf cref l, buf cref r, sz const ln) {
    sz const len = l->len < r->len ? r->len : l->len;
    if (0 == len) return;
    sz first = -1;
    for (sz j = 0; j < ln && j < (len-1)/16+1; j++) {
        printf("%07zx0:   ", j);
        for (sz i = 0; i < 16; i++) {
            sz const k = i+16*j;
            if (l->len <= k) printf("   ");
            else {
                bool const diff = r->len <= k || l->ptr[k] != r->ptr[k];
                if (diff) printf("\x1b[31m");
                printf("%02X ", l->ptr[k]);
                if (diff) printf("\x1b[m");
                if (diff && (sz)-1 == first) first = k;
            }
        }
        printf("       ");
        for (sz i = 0; i < 16; i++) {
            sz const k = i+16*j;
            if (r->len <= k) printf("   ");
            else {
                bool const diff = l->len <= k || l->ptr[k] != r->ptr[k];
                if (diff) printf("\x1b[32m");
                printf("%02X ", r->ptr[k]);
                if (diff) printf("\x1b[m");
                if (diff && (sz)-1 == first) first = k;
            }
        }
        printf("\n");
    }
    if ((sz)-1 != first) printf("first diff at offset %zu\n", first);
}

char const* binstr(u64 n, unsigned const w) {
    static char r[64+3] = {'0', 'b'};
    for (unsigned k = 0; k < w; k++) {
        r[w+1-k] = '0' + (n & 1);
        n>>= 1;
    }
    r[w+2] = '\0';
    return r;
}

void buf_free(buf cref self) {
    if (self->cap) free(self->ptr);
}

buf bufcpy(u8 cref ptr, sz const len) {
    buf r = {.ptr= malloc(len), .len= len, .cap= len};
    if (!r.ptr) exitf("OOM");
    memcpy(r.ptr, ptr, len);
    return r;
}

void bufcat(buf ref to, buf cref other) {
    //u8* dest = dyarr_insert(to, to->len, other->len); // FIXME: this no work, see why
    //if (!dest) exitf("OOM");
    //memcpy(dest, other->ptr, other->len);
    if (!dyarr_resize(to, to->len+other->len)) exitf("OOM");
    memcpy(to->ptr+to->len, other->ptr, other->len);
    to->len+= other->len;
}

u16 peek16le(buf cref b, sz const k) {
    return (u16)b->ptr[k]
         | (u16)b->ptr[k+1]<<8;
}
u32 peek32le(buf cref b, sz const k) {
    return (u32)b->ptr[k]
         | (u32)b->ptr[k+1]<<8
         | (u32)b->ptr[k+2]<<16
         | (u32)b->ptr[k+3]<<24;
}
u64 peek64le(buf cref b, sz const k) {
    return (u64)b->ptr[k]
         | (u64)b->ptr[k+1]<<8
         | (u64)b->ptr[k+2]<<16
         | (u64)b->ptr[k+3]<<24
         | (u64)b->ptr[k+4]<<32
         | (u64)b->ptr[k+5]<<40
         | (u64)b->ptr[k+6]<<48
         | (u64)b->ptr[k+7]<<56;
}

void poke16le(buf ref b, sz const k, u16 const v) {
    b->ptr[k] = v&0xff;
    b->ptr[k+1] = (v>>8)&0xff;
}
void poke32le(buf ref b, sz const k, u32 const v) {
    b->ptr[k] = v&0xff;
    b->ptr[k+1] = (v>>8)&0xff;
    b->ptr[k+2] = (v>>16)&0xff;
    b->ptr[k+3] = (v>>24)&0xff;
}
void poke64le(buf ref b, sz const k, u64 const v) {
    b->ptr[k] = v&0xff;
    b->ptr[k+1] = (v>>8)&0xff;
    b->ptr[k+2] = (v>>16)&0xff;
    b->ptr[k+3] = (v>>24)&0xff;
    b->ptr[k+4] = (v>>32)&0xff;
    b->ptr[k+5] = (v>>40)&0xff;
    b->ptr[k+6] = (v>>48)&0xff;
    b->ptr[k+7] = (v>>56)&0xff;
}

u16 peek16be(buf cref b, sz const k) {
    return (u16)b->ptr[k]<<8
         | (u16)b->ptr[k+1];
}
u32 peek32be(buf cref b, sz const k) {
    return (u32)b->ptr[k]<<24
         | (u32)b->ptr[k+1]<<16
         | (u32)b->ptr[k+2]<<8
         | (u32)b->ptr[k+3];
}
u64 peek64be(buf cref b, sz const k) {
    return (u64)b->ptr[k]<<56
         | (u64)b->ptr[k+1]<<48
         | (u64)b->ptr[k+2]<<40
         | (u64)b->ptr[k+3]<<32
         | (u64)b->ptr[k+4]<<24
         | (u64)b->ptr[k+5]<<16
         | (u64)b->ptr[k+6]<<8
         | (u64)b->ptr[k+7];
}

void poke16be(buf ref b, sz const k, u16 const v) {
    b->ptr[k] = (v>>8)&0xff;
    b->ptr[k+1] = v&0xff;
}
void poke32be(buf ref b, sz const k, u32 const v) {
    b->ptr[k] = (v>>24)&0xff;
    b->ptr[k+1] = (v>>16)&0xff;
    b->ptr[k+2] = (v>>8)&0xff;
    b->ptr[k+3] = v&0xff;
}
void poke64be(buf ref b, sz const k, u64 const v) {
    b->ptr[k] = (v>>56)&0xff;
    b->ptr[k+1] = (v>>48)&0xff;
    b->ptr[k+2] = (v>>40)&0xff;
    b->ptr[k+3] = (v>>32)&0xff;
    b->ptr[k+4] = (v>>24)&0xff;
    b->ptr[k+5] = (v>>16)&0xff;
    b->ptr[k+6] = (v>>8)&0xff;
    b->ptr[k+7] = v&0xff;
}

buf file_read(buf cref path) {
    buf r = {0};
    char local[path->len+1];
    memcpy(local, path->ptr, path->len);
    local[path->len] = '\0';
    FILE* f = fopen(local, "rb");
    if (!f) exitf("could not open file %.*s", (int)path->len, path->ptr);
    if (0 != fseek(f, 0, SEEK_END)) exitf("could not read file %.*s", (int)path->len, path->ptr);
    r.len = ftell(f);
    r.ptr = malloc(r.len);
    if (!r.ptr) exitf("OOM");
    fseek(f, 0, SEEK_SET);
    fread(r.ptr, 1, r.len, f);
    fclose(f);
    return r;
}

void file_write(buf cref path, buf cref b) {
    char local[path->len+1];
    memcpy(local, path->ptr, path->len);
    local[path->len] = '\0';
    FILE* f = fopen(local, "wb");
    if (!f) exitf("could not open file %.*s", (int)path->len, path->ptr);
    fwrite(b->ptr, 1, b->len, f);
    fclose(f);
}

#endif // BIDOOF_IMPLEMENTATION

#ifdef BIDOOF_LIST_DEPS

struct _list_deps_item { struct _list_deps_item cref next; char cref name; };
#define _list_deps_first NULL
#define make_main(...)                                             \
    int main(void) {                                               \
        for ( struct _list_deps_item const* it = _list_deps_first  \
            ; it                                                   \
            ; it = it->next                                        \
            ) puts(it->name);                                      \
        return 0;                                                  \
    }

#else // BIDOOF_LIST_DEPS

#ifdef BIDOOF_T_IMPLEMENTATION
#define BIDOOF_IMPLEMENTATION
#endif

#define make_arg_buf(__name, __information)                                                  \
    buf cref __name = (_is_h ? puts("\t" #__name ":\t" __information), NULL                  \
                    : !argc ? exitf("expected value for argument '" #__name "'"), NULL       \
                    : (argc--, argv++, &(buf){.ptr= (u8*)argv[-1], .len= strlen(argv[-1])})  \
                    )
#define make_arg_int(__name, __information)                                           \
    int const __name = (_is_h ? puts("\t" #__name ":\t" __information), 0             \
                    : !argc ? exitf("expected number for argument '" #__name "'"), 0  \
                    : (argc--, atoi(*argv++))                                         \
                    )

#define make_cmd(__invocation, __description, ...) do                             \
    if (_is_h) puts("\t" #__invocation ":\t" __description);                      \
    else {                                                                        \
        static char const invocation[] = #__invocation;                           \
        if (!strncmp(invocation, *argv, strchr(invocation, '(') - invocation)) {  \
            argc--, argv++;                                                       \
            if (argc && !strcmp("-h", *argv)) {                                   \
                puts(#__invocation ":\t" __description);                          \
                static bool const _is_h = true;                                   \
                __VA_ARGS__                                                       \
                return 0;                                                         \
            }                                                                     \
            static bool const _is_h = false;                                      \
            __VA_ARGS__                                                           \
            __invocation;                                                         \
            return 0;                                                             \
        }                                                                         \
    } while (0)

#define make_main(__summary, ...)                  \
    int main(int argc, char** argv) {              \
        char cref proc = (argc--, *argv++);        \
        if (!argc || !strcmp("-h", *argv)) {       \
            printf("%s: " __summary "\n", proc);   \
            static bool const _is_h = true;        \
            __VA_ARGS__                            \
            return 0;                              \
        }                                          \
        static bool const _is_h = false;           \
        __VA_ARGS__                                \
        printf("unknown command: '%s'\n", *argv);  \
        return 1;                                  \
    }

#endif // BIDOOF_LIST_DEPS

#endif // __BIDOOF_H__

#undef adapt_bipa_type
#ifdef BIDOOF_IMPLEMENTATION
#define adapt_bipa_type(__tname)                   \
    typedef struct __tname __tname;                \
    void __tname##_dump(__tname cref self) {       \
        bipa_dump_##__tname(self, stdout, 0);      \
        putchar('\n');                             \
    }                                              \
    buf __tname##_build(__tname cref self) {       \
        buf r = {0};                               \
        if (!bipa_build_##__tname(self, &r))       \
            exitf("could not build a " #__tname);  \
        return r;                                  \
    }                                              \
    __tname __tname##_parse(buf cref buf) {        \
        struct __tname r;                          \
        sz at = 0;                                 \
        if (!bipa_parse_##__tname(&r, buf, &at))   \
            exitf("could not parse a " #__tname);  \
        return r;                                  \
    }                                              \
    void __tname##_free(__tname cref self) {       \
        bipa_free_##__tname(self);                 \
    }
#else
#define adapt_bipa_type(__tname)                   \
    typedef struct __tname __tname;                \
    void __tname##_dump(__tname cref self);        \
    buf __tname##_build(__tname cref self);        \
    __tname __tname##_parse(buf cref buf);         \
    void __tname##_free(__tname cref self);
#endif
