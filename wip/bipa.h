/// in an (array, typename, while), the while is an expression with:
///  - `self` the encompassing struct
///  - `k` the iteration number
///  - `it` the array itself (so `it->ptr[k-1]` the last parsed item, `it->ptr[k]` is not parsed yet)
/// a `build` or `parse` macro is a sequence of statements with:
///  - `it` is the source (resp. destination) pointer
///  - `bi` (resp. `pa`) is the builder (parser)
///  - writing a byte: `bi->arr.ptr[bi->at++] = *it`
///  - reading a byte: `*it = pa->buf->ptr[pa->at++]`

#include <stdint.h>
#include <stdio.h>

#include "../dyarr.h"

typedef struct BufBuilder {
    dyarr(u8) arr;
} BufBuilder;

typedef struct BufParser {
    Buf const* buf;
    sz at;
    //bool fail;
    //bool end;
} BufParser;

#ifdef _BIPA_HIDUMP
#define _hidump_kw(__c) "\x1b[34m" __c "\x1b[m"
#define _hidump_ty(__c) "\x1b[32m" __c "\x1b[m"
#define _hidump_nb(__c) "\x1b[33m" __c "\x1b[m"
#define _hidump_nx(__c) "\x1b[35m" __c "\x1b[m"
#define _hidump_id(__c) __c
#else // _BIPA_HIDUMP
#define _hidump_kw(__c) __c
#define _hidump_ty(__c) __c
#define _hidump_nb(__c) __c
#define _hidump_nx(__c) __c
#define _hidump_id(__c) __c
#endif // _BIPA_HIDUMP

#define _expand(__m, ...) __m(__VA_ARGS__)

#define _typename(__ty, ...) _expand(_typename_##__ty, __VA_ARGS__)
#define _dump(__ty, ...) _expand(_dump_##__ty, __VA_ARGS__)
#define _build(__ty, ...) _expand(_build_##__ty, __VA_ARGS__)
#define _parse(__ty, ...) _expand(_parse_##__ty, __VA_ARGS__)

#define _typename_u8() uint8_t
#define _dump_u8()  printf(_hidump_nb("%hhu") _hidump_nx("u8"), *it);
#define _build_u8() uint8_t* p = dyarr_push(&bi->arr);  \
                    if (!p) goto fail;  \
                    *p = *it;
#define _parse_u8() if (pa->buf->len == pa->at) goto fail;  \
                    *it = pa->buf->ptr[pa->at++];

#define _typename_u16le() uint16_t
#define _typename_u32le() uint32_t
#define _typename_u64le() uint64_t
#define _dump_u16le()   printf(_hidump_nb("%hu") _hidump_nx("u16le"), *it);
#define _dump_u32le()   printf(_hidump_nb("%u" ) _hidump_nx("u32le"), *it);
#define _dump_u64le()   printf(_hidump_nb("%lu") _hidump_nx("u64le"), *it);
#define _build_u16le()  if (bi->arr.cap <= bi->arr.len+1 &&                    \
                            !dyarr_resize(&bi->arr,                            \
                                bi->arr.cap ? bi->arr.cap*2 : 16)) goto fail;  \
                        bi->arr.ptr[bi->arr.len++] = *it&0xff;                 \
                        bi->arr.ptr[bi->arr.len++] = (*it>>8)&0xff;
#define _build_u32le()  if (bi->arr.cap <= bi->arr.len+3 &&                    \
                            !dyarr_resize(&bi->arr,                            \
                                bi->arr.cap ? bi->arr.cap*2 : 16)) goto fail;  \
                        bi->arr.ptr[bi->arr.len++] = *it&0xff;                 \
                        bi->arr.ptr[bi->arr.len++] = (*it>>8)&0xff;            \
                        bi->arr.ptr[bi->arr.len++] = (*it>>16)&0xff;           \
                        bi->arr.ptr[bi->arr.len++] = (*it>>24)&0xff;
#define _build_u64le()  if (bi->arr.cap <= bi->arr.len+7 &&                    \
                            !dyarr_resize(&bi->arr,                            \
                                bi->arr.cap ? bi->arr.cap*2 : 16)) goto fail;  \
                        bi->arr.ptr[bi->arr.len++] = *it&0xff;                 \
                        bi->arr.ptr[bi->arr.len++] = (*it>>8)&0xff;            \
                        bi->arr.ptr[bi->arr.len++] = (*it>>16)&0xff;           \
                        bi->arr.ptr[bi->arr.len++] = (*it>>24)&0xff;           \
                        bi->arr.ptr[bi->arr.len++] = (*it>>32)&0xff;           \
                        bi->arr.ptr[bi->arr.len++] = (*it>>40)&0xff;           \
                        bi->arr.ptr[bi->arr.len++] = (*it>>48)&0xff;           \
                        bi->arr.ptr[bi->arr.len++] = (*it>>56)&0xff;
#define _parse_u16le()  if (pa->buf->len <= pa->at+1) goto fail;               \
                        *it = pa->buf->ptr[pa->at]                             \
                            | pa->buf->ptr[pa->at+1]<<8;                       \
                        pa->at+= 2;
#define _parse_u32le()  if (pa->buf->len <= pa->at+3) goto fail;               \
                        *it = pa->buf->ptr[pa->at]                             \
                            | pa->buf->ptr[pa->at+1]<<8                        \
                            | pa->buf->ptr[pa->at+2]<<16                       \
                            | pa->buf->ptr[pa->at+3]<<24;                      \
                        pa->at+= 4;
#define _parse_u64le()  if (pa->buf->len <= pa->at+7) goto fail;               \
                        *it = pa->buf->ptr[pa->at]                             \
                            | pa->buf->ptr[pa->at+1]<<8                        \
                            | pa->buf->ptr[pa->at+2]<<16                       \
                            | pa->buf->ptr[pa->at+3]<<24;                      \
                            | pa->buf->ptr[pa->at+4]<<32;                      \
                            | pa->buf->ptr[pa->at+5]<<40;                      \
                            | pa->buf->ptr[pa->at+6]<<48;                      \
                            | pa->buf->ptr[pa->at+7]<<56;                      \
                        pa->at+= 8;

#define _typename_u16be() uint16_t
#define _typename_u32be() uint32_t
#define _typename_u64be() uint64_t
#define _dump_u16be()   printf(_hidump_nb("%hu") _hidump_nx("u16be"), *it);
#define _dump_u32be()   printf(_hidump_nb("%u" ) _hidump_nx("u32be"), *it);
#define _dump_u64be()   printf(_hidump_nb("%lu") _hidump_nx("u64be"), *it);
#define _build_u16be()  if (bi->arr.cap <= bi->arr.len+1 &&                     \
                            !dyarr_resize(&bi->arr,                             \
                                bi->arr.cap ? bi->arr.cap*2 : 16)) goto fail;   \
                        bi->arr.ptr[bi->arr.len++] = (*it>>8)&0xff;             \
                        bi->arr.ptr[bi->arr.len++] = *it&0xff;
#define _build_u32be()  if (bi->arr.cap <= bi->arr.len+3 &&                     \
                            !dyarr_resize(&bi->arr,                             \
                                bi->arr.cap ? bi->arr.cap*2 : 16)) goto fail;   \
                        bi->arr.ptr[bi->arr.len++] = (*it>>24)&0xff;            \
                        bi->arr.ptr[bi->arr.len++] = (*it>>16)&0xff;            \
                        bi->arr.ptr[bi->arr.len++] = (*it>>8)&0xff;             \
                        bi->arr.ptr[bi->arr.len++] = *it&0xff;
#define _build_u64be()  if (bi->arr.cap <= bi->arr.len+7 &&                     \
                            !dyarr_resize(&bi->arr,                             \
                                bi->arr.cap ? bi->arr.cap*2 : 16)) goto fail;   \
                        bi->arr.ptr[bi->arr.len++] = (*it>>56)&0xff;            \
                        bi->arr.ptr[bi->arr.len++] = (*it>>48)&0xff;            \
                        bi->arr.ptr[bi->arr.len++] = (*it>>40)&0xff;            \
                        bi->arr.ptr[bi->arr.len++] = (*it>>32)&0xff;            \
                        bi->arr.ptr[bi->arr.len++] = (*it>>24)&0xff;            \
                        bi->arr.ptr[bi->arr.len++] = (*it>>16)&0xff;            \
                        bi->arr.ptr[bi->arr.len++] = (*it>>8)&0xff;             \
                        bi->arr.ptr[bi->arr.len++] = *it&0xff;
#define _parse_u16be()  if (pa->buf->len <= pa->at+1) goto fail;                \
                        *it = pa->buf->ptr[pa->at+1]                            \
                            | pa->buf->ptr[pa->at]<<8;                          \
                        pa->at+= 2;
#define _parse_u32be()  if (pa->buf->len <= pa->at+3) goto fail;                \
                        *it = pa->buf->ptr[pa->at+3]                            \
                            | pa->buf->ptr[pa->at+2]<<8                         \
                            | pa->buf->ptr[pa->at+1]<<16                        \
                            | pa->buf->ptr[pa->at]<<24;                         \
                        pa->at+= 4;
#define _parse_u64be()  if (pa->buf->len <= pa->at+7) goto fail;                \
                        *it = pa->buf->ptr[pa->at+7]                            \
                            | pa->buf->ptr[pa->at+6]<<8                         \
                            | pa->buf->ptr[pa->at+5]<<16                        \
                            | pa->buf->ptr[pa->at+4]<<24;                       \
                            | pa->buf->ptr[pa->at+3]<<32;                       \
                            | pa->buf->ptr[pa->at+2]<<40;                       \
                            | pa->buf->ptr[pa->at+1]<<48;                       \
                            | pa->buf->ptr[pa->at]<<56;                         \
                        pa->at+= 8;

/// TODO: cstr/lstr

#define bipa_struct(__tname, __ty1, __nm1, __ty2, __nm2)                            \
    struct __tname {                                                                \
        _typename __ty1 __nm1;                                                      \
        _typename __ty2 __nm2;                                                      \
    };                                                                              \
    void bipa_dump_##__tname(struct __tname const* const self) {                    \
        printf(_hidump_kw("struct") " " _hidump_ty(#__tname) " {");                 \
        bool _f = true;                                                             \
        do {                                                                        \
            if (_f) _f = false; else printf(", ");                                  \
            printf("." _hidump_id(#__nm1) "= ");                                    \
            _typename __ty1 const* const it = &self->__nm1;                         \
            _dump __ty1                                                             \
        } while (0);                                                                \
        do {                                                                        \
            if (_f) _f = false; else printf(", ");                                  \
            printf("." _hidump_id(#__nm2) "= ");                                    \
            _typename __ty2 const* const it = &self->__nm2;                         \
            _dump __ty2                                                             \
        } while (0);                                                                \
        printf("}");                                                                \
    }                                                                               \
    static inline bool                                                              \
    bipa_build_##__tname(struct __tname const* const self, BufBuilder* const bi) {  \
        do {                                                                        \
            _typename __ty1 const* const it = &self->__nm1;                         \
            _build __ty1                                                            \
        } while (0);                                                                \
        do {                                                                        \
            _typename __ty2 const* const it = &self->__nm2;                         \
            _build __ty2                                                            \
        } while (0);                                                                \
        return true;                                                                \
    fail: return false;                                                             \
    }                                                                               \
    static inline bool                                                              \
    bipa_parse_##__tname(struct __tname* const self, BufParser* const pa) {         \
        size_t at_before = pa->at;                                                  \
        do {                                                                        \
            _typename __ty1* const it = &self->__nm1;                               \
            _parse __ty1                                                            \
        } while (0);                                                                \
        do {                                                                        \
            _typename __ty2* const it = &self->__nm2;                               \
            _parse __ty2                                                            \
        } while (0);                                                                \
        return true;                                                                \
    fail:                                                                           \
        pa->at = at_before;                                                         \
        return false;                                                               \
    }
#define _typename_struct(__tname) struct __tname
#define _dump_struct(__tname) bipa_dump_##__tname(it);
#define _build_struct(__tname) if (!bipa_build_##__tname(it, bi)) goto fail;
#define _parse_struct(__tname) if (!bipa_parse_##__tname(it, pa)) goto fail;

/// TODO: bipa_union, bipa_enum

#define bipa_option(__tname, __of)                                                  \
    struct __tname {                                                                \
        _typename __of val;                                                         \
        bool has;                                                                   \
    };                                                                              \
    void bipa_dump_##__tname(struct __tname const* const self) {                    \
        printf(_hi_dump_kw("option") " " _hidump_ty(#__tname) " ");                 \
        if (self->has) {                                                            \
            _typename __of const* const it = &self->val;                            \
            _dump __of                                                              \
        } else printf(_hidump_kw("nil"));                                           \
    }                                                                               \
    static inline bool                                                              \
    bipa_build_##__tname(struct __tname const* const self, BufBuilder* const bi) {  \
        if (self->has) {                                                            \
            _typename __of const* const it = &self->val;                            \
            _build __of                                                             \
        } else (void)0;                                                             \
        return true;                                                                \
    fail: return false;                                                             \
    }                                                                               \
    static inline bool                                                              \
    bipa_parse_has_##__tname(struct __tname* const self, BufParser* const pa) {     \
        size_t at_before = pa->at;                                                  \
        _typename __of* const it = dyarr_push(self);                                \
        _parse __of                                                                 \
        return true;                                                                \
    fail:                                                                           \
        pa->at = at_before;                                                         \
        return false;                                                               \
    }
#define _typename_option(__tname) struct __tname
#define _dump_option(__tname, _) bipa_dump_##__tname(it);
#define _build_option(__tname, _) if (!bipa_build_##__tname(it, bi)) goto fail;
#define _parse_option(__tname, __if) if (__if) if (!bipa_parse_has_##__tname(it, pa)) goto fail;

#define bipa_array(__tname, __of)                                                   \
    struct __tname {                                                                \
        _typename __of* ptr;                                                        \
        size_t len, cap;                                                            \
    };                                                                              \
    void bipa_dump_##__tname(struct __tname const* const self) {                    \
        printf(_hidump_kw("array") " " _hidump_ty(#__tname) " [");                  \
        bool _f = true;                                                             \
        for (size_t k = 0; k < self->len; k++) {                                    \
            if (_f) _f = false; else printf(", ");                                  \
            _typename __of const* const it = self->ptr + k;                         \
            _dump __of                                                              \
        }                                                                           \
        printf("]");                                                                \
    }                                                                               \
    static inline bool                                                              \
    bipa_build_##__tname(struct __tname const* const self, BufBuilder* const bi) {  \
        for (size_t k = 0; k < self->len; k++) {                                    \
            _typename __of const* const it = self->ptr + k;                         \
            _build __of                                                             \
        }                                                                           \
        return true;                                                                \
    fail: return false;                                                             \
    }                                                                               \
    static inline bool                                                              \
    bipa_parse_one_##__tname(struct __tname* const self, BufParser* const pa) {     \
        size_t at_before = pa->at;                                                  \
        _typename __of* const it = dyarr_push(self);                                \
        _parse __of                                                                 \
        return true;                                                                \
    fail:                                                                           \
        pa->at = at_before;                                                         \
        return false;                                                               \
    }
#define _typename_array(__tname, _) struct __tname
#define _dump_array(__tname, _) bipa_dump_##__tname(it);
#define _build_array(__tname, _) if (!bipa_build_##__tname(it, bi)) goto fail;
#define _parse_array(__tname, __while) for (size_t k = 0; __while; k++) if (!bipa_parse_one_##__tname(it, pa)) goto fail;
