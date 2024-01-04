#ifndef __BIPA_H__
#define __BIPA_H__

/// in an (array, typename, while), the while is an expression with:
///  - `self` the encompassing struct
///  - `k` the iteration number
///  - `it` the array itself (so `it->ptr[k-1]` the last parsed item, `it->ptr[k]` is not parsed yet)
/// a `build` or `parse` macro is a sequence of statements with:
///  - `it` is the source (resp. destination) pointer
///  - `bi` (resp. `pa`) is the builder (parser)
///  - writing a byte: `bi->arr.ptr[bi->at++] = *it`
///  - reading a byte: `*it = pa->buf->ptr[pa->at++]`

/// TODO: _free, ie for each types

#include "base.h"

typedef struct BufBuilder {
    dyarr(u8) arr;
} BufBuilder;

typedef struct BufParser {
    Buf const* buf;
    sz at;
    //bool fail;
    //bool end;
} BufParser;

#ifdef BIPA_HIDUMP
#define _hidump_kw(__c) "\x1b[34m" __c "\x1b[m"
#define _hidump_ty(__c) "\x1b[32m" __c "\x1b[m"
#define _hidump_nb(__c) "\x1b[33m" __c "\x1b[m"
#define _hidump_st(__c) "\x1b[36m" __c "\x1b[m"
#define _hidump_ex(__c) "\x1b[35m" __c "\x1b[m"
#define _hidump_id(__c) __c
#else // BIPA_HIDUMP
#define _hidump_kw(__c) __c
#define _hidump_ty(__c) __c
#define _hidump_nb(__c) __c
#define _hidump_st(__c) __c
#define _hidump_ex(__c) __c
#define _hidump_id(__c) __c
#endif // BIPA_HIDUMP

#define _typename(__ty, ...) _CALL(_typename_##__ty, __VA_ARGS__)
#define _dump(__ty, ...)     _CALL(_dump_##__ty, __VA_ARGS__)
#define _build(__ty, ...)    _CALL(_build_##__ty, __VA_ARGS__)
#define _parse(__ty, ...)    _CALL(_parse_##__ty, __VA_ARGS__)

#define _typename_void() bool
#define _dump_void()  (void)it; printf(_hidump_kw("void"));
#define _build_void() (void)it;
#define _parse_void() (void)it;

#define _typename_u8() uint8_t
#define _dump_u8()  printf(_hidump_nb("%hhu") _hidump_ex("u8"), *it);
#define _build_u8() { uint8_t* p = dyarr_push(&bi->arr);  \
                    if (!p) goto fail;  \
                    *p = *it; }
#define _parse_u8() if (pa->buf->len == pa->at) goto fail;  \
                    *it = pa->buf->ptr[pa->at++];

#define _typename_u16le() uint16_t
#define _typename_u32le() uint32_t
#define _typename_u64le() uint64_t
#define _dump_u16le()   printf(_hidump_nb("%hu") _hidump_ex("u16le"), *it);
#define _dump_u32le()   printf(_hidump_nb("%u" ) _hidump_ex("u32le"), *it);
#define _dump_u64le()   printf(_hidump_nb("%lu") _hidump_ex("u64le"), *it);
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
                        *it = (uint16_t)pa->buf->ptr[pa->at]                   \
                            | (uint16_t)pa->buf->ptr[pa->at+1]<<8;             \
                        pa->at+= 2;
#define _parse_u32le()  if (pa->buf->len <= pa->at+3) goto fail;               \
                        *it = (uint32_t)pa->buf->ptr[pa->at]                   \
                            | (uint32_t)pa->buf->ptr[pa->at+1]<<8              \
                            | (uint32_t)pa->buf->ptr[pa->at+2]<<16             \
                            | (uint32_t)pa->buf->ptr[pa->at+3]<<24;            \
                        pa->at+= 4;
#define _parse_u64le()  if (pa->buf->len <= pa->at+7) goto fail;               \
                        *it = (uint64_t)pa->buf->ptr[pa->at]                   \
                            | (uint64_t)pa->buf->ptr[pa->at+1]<<8              \
                            | (uint64_t)pa->buf->ptr[pa->at+2]<<16             \
                            | (uint64_t)pa->buf->ptr[pa->at+3]<<24             \
                            | (uint64_t)pa->buf->ptr[pa->at+4]<<32             \
                            | (uint64_t)pa->buf->ptr[pa->at+5]<<40             \
                            | (uint64_t)pa->buf->ptr[pa->at+6]<<48             \
                            | (uint64_t)pa->buf->ptr[pa->at+7]<<56;            \
                        pa->at+= 8;

#define _typename_u16be() uint16_t
#define _typename_u32be() uint32_t
#define _typename_u64be() uint64_t
#define _dump_u16be()   printf(_hidump_nb("%hu") _hidump_ex("u16be"), *it);
#define _dump_u32be()   printf(_hidump_nb("%u" ) _hidump_ex("u32be"), *it);
#define _dump_u64be()   printf(_hidump_nb("%lu") _hidump_ex("u64be"), *it);
#define _build_u16be()  if (bi->arr.cap <= bi->arr.len+1 &&                    \
                            !dyarr_resize(&bi->arr,                            \
                                bi->arr.cap ? bi->arr.cap*2 : 16)) goto fail;  \
                        bi->arr.ptr[bi->arr.len++] = (*it>>8)&0xff;            \
                        bi->arr.ptr[bi->arr.len++] = *it&0xff;
#define _build_u32be()  if (bi->arr.cap <= bi->arr.len+3 &&                    \
                            !dyarr_resize(&bi->arr,                            \
                                bi->arr.cap ? bi->arr.cap*2 : 16)) goto fail;  \
                        bi->arr.ptr[bi->arr.len++] = (*it>>24)&0xff;           \
                        bi->arr.ptr[bi->arr.len++] = (*it>>16)&0xff;           \
                        bi->arr.ptr[bi->arr.len++] = (*it>>8)&0xff;            \
                        bi->arr.ptr[bi->arr.len++] = *it&0xff;
#define _build_u64be()  if (bi->arr.cap <= bi->arr.len+7 &&                    \
                            !dyarr_resize(&bi->arr,                            \
                                bi->arr.cap ? bi->arr.cap*2 : 16)) goto fail;  \
                        bi->arr.ptr[bi->arr.len++] = (*it>>56)&0xff;           \
                        bi->arr.ptr[bi->arr.len++] = (*it>>48)&0xff;           \
                        bi->arr.ptr[bi->arr.len++] = (*it>>40)&0xff;           \
                        bi->arr.ptr[bi->arr.len++] = (*it>>32)&0xff;           \
                        bi->arr.ptr[bi->arr.len++] = (*it>>24)&0xff;           \
                        bi->arr.ptr[bi->arr.len++] = (*it>>16)&0xff;           \
                        bi->arr.ptr[bi->arr.len++] = (*it>>8)&0xff;            \
                        bi->arr.ptr[bi->arr.len++] = *it&0xff;
#define _parse_u16be()  if (pa->buf->len <= pa->at+1) goto fail;               \
                        *it = (uint16_t)pa->buf->ptr[pa->at+1]                 \
                            | (uint16_t)pa->buf->ptr[pa->at]<<8;               \
                        pa->at+= 2;
#define _parse_u32be()  if (pa->buf->len <= pa->at+3) goto fail;               \
                        *it = (uint32_t)pa->buf->ptr[pa->at+3]                 \
                            | (uint32_t)pa->buf->ptr[pa->at+2]<<8              \
                            | (uint32_t)pa->buf->ptr[pa->at+1]<<16             \
                            | (uint32_t)pa->buf->ptr[pa->at]<<24;              \
                        pa->at+= 4;
#define _parse_u64be()  if (pa->buf->len <= pa->at+7) goto fail;               \
                        *it = (uint64_t)pa->buf->ptr[pa->at+7]                 \
                            | (uint64_t)pa->buf->ptr[pa->at+6]<<8              \
                            | (uint64_t)pa->buf->ptr[pa->at+5]<<16             \
                            | (uint64_t)pa->buf->ptr[pa->at+4]<<24             \
                            | (uint64_t)pa->buf->ptr[pa->at+3]<<32             \
                            | (uint64_t)pa->buf->ptr[pa->at+2]<<40             \
                            | (uint64_t)pa->buf->ptr[pa->at+1]<<48             \
                            | (uint64_t)pa->buf->ptr[pa->at]<<56;              \
                        pa->at+= 8;

#define _typename_cstr(__sentinel) uint8_t*
#define _dump_cstr(__sentinel) {                                  \
        uint8_t s = (__sentinel);                                 \
        for (size_t k = 0; s != (*it)[k]; k++) {                  \
            if (!(k & 0xf)) printf("\n%*.s", (depth+2)*2-1, "");  \
            printf(" " _hidump_st("%02X"), (*it)[k]);             \
        }                                                         \
        printf(_hidump_ex(" \\x%02X"), s);                        \
    }
#define _build_cstr(__sentinel) {                                  \
        uint8_t s = (__sentinel);                                  \
        size_t len = (uint8_t*)strchr((char*)*it, s) - *it;        \
        while (bi->arr.cap <= bi->arr.len+len+1)                   \
            if (!dyarr_resize(&bi->arr,                            \
                    bi->arr.cap ? bi->arr.cap*2 : 16)) goto fail;  \
        memcpy(bi->arr.ptr+bi->arr.len, *it, len);                 \
        bi->arr.len+= len;                                         \
        bi->arr.ptr[bi->arr.len++] = s;                            \
    }
#define _parse_cstr(__sentinel) {                                          \
        uint8_t* from = pa->buf->ptr+pa->at;                               \
        uint8_t* found = memchr(from, (__sentinel), pa->buf->len-pa->at);  \
        if (!found) goto fail;                                             \
        size_t len = found-from;                                           \
        *it = malloc(len);                                                 \
        if (!*it) goto fail;                                               \
        memcpy(*it, from, len);                                            \
        pa->at+= len+1;                                                    \
    }

#define _typename_lstr(__length) uint8_t*
#define _dump_lstr(__length) {                                    \
        size_t len = (__length);                                  \
        for (size_t k = 0; k < len; k++) {                        \
            if (!(k & 0xf)) printf("\n%*.s", (depth+2)*2-1, "");  \
            printf(" " _hidump_st("%02X"), (*it)[k]);             \
        }                                                         \
    }
#define _build_lstr(__length) {                                    \
        size_t len = (__length);                                   \
        while (bi->arr.cap <= bi->arr.len+len+1)                   \
            if (!dyarr_resize(&bi->arr,                            \
                    bi->arr.cap ? bi->arr.cap*2 : 16)) goto fail;  \
        memcpy(bi->arr.ptr+bi->arr.len, *it, len);                 \
        bi->arr.len+= len;                                         \
    }
#define _parse_lstr(__length) {                    \
        uint8_t* from = pa->buf->ptr+pa->at;       \
        size_t len = (__length);                   \
        if (pa->buf->len < pa->at+len) goto fail;  \
        *it = malloc(len);                         \
        if (!*it) goto fail;                       \
        memcpy(*it, from, len);                    \
        pa->at+= len;                              \
    }

#define _struct_fields_typename_one(__k, __n, __inv, __ty, __nm) _typename __ty __nm;

#define _struct_fields_dump_one(__k, __n, __inv, __ty, __nm) {  \
        printf("." _hidump_id(#__nm) "= ");                     \
        _typename __ty const* const it = &self->__nm;           \
        _dump __ty                                              \
        printf(",\n%*.s", (depth+(__k+1!=__n))*2, "");          \
    }

#define _struct_fields_build_one(__k, __n, __inv, __ty, __nm) {  \
        _typename __ty const* const it = &self->__nm;            \
        _build __ty                                              \
    }

#define _struct_fields_parse_one(__k, __n, __inv, __ty, __nm) {  \
        _typename __ty* const it = &self->__nm;                  \
        _parse __ty                                              \
    }

#define bipa_struct(__tname, __n_fields, ...)                                       \
    struct __tname {                                                                \
        _FOR_TYNM(__n_fields, _struct_fields_typename_one, 0, __VA_ARGS__)          \
    };                                                                              \
    void bipa_dump_##__tname(struct __tname const* const self, int const depth) {   \
        (void)depth;                                                                \
        printf(_hidump_kw("struct") " " _hidump_ty(#__tname) " {\n%*.s",            \
                (depth+1)*2, "");                                                   \
        _FOR_TYNM(__n_fields, _struct_fields_dump_one, 0, __VA_ARGS__)              \
        printf("}");                                                                \
    }                                                                               \
    static inline bool                                                              \
    bipa_build_##__tname(struct __tname const* const self, BufBuilder* const bi) {  \
        _FOR_TYNM(__n_fields, _struct_fields_build_one, 0, __VA_ARGS__)             \
        return true;                                                                \
    fail: return false;                                                             \
    }                                                                               \
    static inline bool                                                              \
    bipa_parse_##__tname(struct __tname* const self, BufParser* const pa) {         \
        size_t at_before = pa->at;                                                  \
        _FOR_TYNM(__n_fields, _struct_fields_parse_one, 0, __VA_ARGS__)             \
        return true;                                                                \
    fail:                                                                           \
        pa->at = at_before;                                                         \
        return false;                                                               \
    }
#define _typename_struct(__tname) struct __tname
#define _dump_struct(__tname) bipa_dump_##__tname(it, depth+1);
#define _build_struct(__tname) if (!bipa_build_##__tname(it, bi)) goto fail;
#define _parse_struct(__tname) if (!bipa_parse_##__tname(it, pa)) goto fail;

#define _union_getvar_name(__tty, __tva, __nm)     __nm
#define _union_getvar_tagtype(__tty, __tva, __nm)  __tty
#define _union_getvar_tagvalue(__tty, __tva, __nm) __tva

#define _union_kinds_typename_one(__k, __n, __inv, __ty, __nm) _typename __ty _union_getvar_name __nm;
#define _union_kinds_tagname_one(__k, __n, __tname, __ty, __nm) _XJOIN(__tname##_tag_, _union_getvar_name __nm) = __k,

#define _union_kinds_dump_one(__k, __n, __tname, __ty, __nm)  \
    case _XJOIN(__tname##_tag_, _union_getvar_name __nm): {   \
        printf("|");                                          \
        {                                                     \
            _XJOIN(_typename_, _union_getvar_tagtype __nm)()  \
                const _it = _union_getvar_tagvalue __nm,      \
                * const it = &_it;                            \
            _XJOIN(_dump_, _union_getvar_tagtype __nm)()      \
        }                                                     \
        printf("| ");                                         \
        {                                                     \
            _typename __ty const* const it =                  \
                &self->val._union_getvar_name __nm;           \
            _dump __ty                                        \
        }                                                     \
    } break;

#define _union_kinds_build_one(__k, __n, __tname, __ty, __nm)  \
    case _XJOIN(__tname##_tag_, _union_getvar_name __nm): {    \
        {                                                      \
            _XJOIN(_typename_, _union_getvar_tagtype __nm)()   \
                const _it = _union_getvar_tagvalue __nm,       \
                * const it = &_it;                             \
            _XJOIN(_build_, _union_getvar_tagtype __nm)()      \
        }                                                      \
        {                                                      \
            _typename __ty const* const it =                   \
                &self->val._union_getvar_name __nm;            \
            _build __ty                                        \
        }                                                      \
    } break;

#define _union_kinds_parse_one(__k, __n, __tname, __ty, __nm)   \
    case _XJOIN(__tname##_tag_, _union_getvar_name __nm): {     \
        {                                                       \
            _XJOIN(_typename_, _union_getvar_tagtype __nm)()    \
                _it = 0,                                        \
                * const it = &_it;                              \
            _XJOIN(_parse_, _union_getvar_tagtype __nm)()       \
            if (_union_getvar_tagvalue __nm != _it) goto fail;  \
        }                                                       \
        {                                                       \
            _typename __ty* const it =                          \
                &self->val._union_getvar_name __nm;             \
            _parse __ty                                         \
        }                                                       \
        self->tag = _k_kinds;                                   \
    } break;

#define bipa_union(__tname, __n_kinds, ...)                                         \
    struct __tname {                                                                \
        union {                                                                     \
            _FOR_TYNM(__n_kinds, _union_kinds_typename_one, 0, __VA_ARGS__)         \
        } val;                                                                      \
        enum {                                                                      \
            _FOR_TYNM(__n_kinds, _union_kinds_tagname_one, __tname, __VA_ARGS__)    \
        } tag;                                                                      \
    };                                                                              \
    void bipa_dump_##__tname(struct __tname const* const self, int const depth) {   \
        (void)depth;                                                                \
        switch (self->tag) {                                                        \
            _FOR_TYNM(__n_kinds, _union_kinds_dump_one, __tname, __VA_ARGS__)       \
            default: printf("|erroneous|");                                         \
        }                                                                           \
    }                                                                               \
    static inline bool                                                              \
    bipa_build_##__tname(struct __tname const* const self, BufBuilder* const bi) {  \
        switch (self->tag) {                                                        \
            _FOR_TYNM(__n_kinds, _union_kinds_build_one, __tname, __VA_ARGS__)      \
            default: goto fail;                                                     \
        }                                                                           \
        return true;                                                                \
    fail: return false;                                                             \
    }                                                                               \
    static inline bool                                                              \
    bipa_parse_##__tname(struct __tname* const self, BufParser* const pa) {         \
        size_t at_before = pa->at;                                                  \
        for (size_t _k_kinds = 0; _k_kinds < __n_kinds; _k_kinds++) {               \
            switch (_k_kinds) {                                                     \
                _FOR_TYNM(__n_kinds, _union_kinds_parse_one, __tname, __VA_ARGS__)  \
            }                                                                       \
            return true;                                                            \
        fail: pa->at = at_before;                                                   \
        }                                                                           \
        return false;                                                               \
    }
#define _typename_union(__tname) struct __tname
#define _dump_union(__tname) bipa_dump_##__tname(it, depth+1);
#define _build_union(__tname) if (!bipa_build_##__tname(it, bi)) goto fail;
#define _parse_union(__tname) if (!bipa_parse_##__tname(it, pa)) goto fail;

#define bipa_option(__tname, __of)                                                  \
    struct __tname {                                                                \
        _typename __of val;                                                         \
        bool has;                                                                   \
    };                                                                              \
    void bipa_dump_##__tname(struct __tname const* const self, int const depth) {   \
        (void)depth;                                                                \
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
#define _dump_option(__tname, _) bipa_dump_##__tname(it, depth+1);
#define _build_option(__tname, _) if (!bipa_build_##__tname(it, bi)) goto fail;
#define _parse_option(__tname, __if) if (__if) if (!bipa_parse_has_##__tname(it, pa)) goto fail;

#define bipa_array(__tname, __of)                                                   \
    struct __tname {                                                                \
        _typename __of* ptr;                                                        \
        size_t len, cap;                                                            \
    };                                                                              \
    void bipa_dump_##__tname(struct __tname const* const self, int const depth) {   \
        (void)depth;                                                                \
        printf(_hidump_kw("array") " " _hidump_ty(#__tname) " [\n%*.s",             \
                (depth+1)*2, "");                                                   \
        for (size_t k = 0; k < self->len; k++) {                                    \
            _typename __of const* const it = self->ptr + k;                         \
            _dump __of                                                              \
            printf(",\n%*.s", (depth+(k+1!=self->len))*2, "");                      \
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
#define _dump_array(__tname, _) bipa_dump_##__tname(it, depth+1);
#define _build_array(__tname, _) if (!bipa_build_##__tname(it, bi)) goto fail;
#define _parse_array(__tname, __while) for (size_t k = 0; __while; k++) if (!bipa_parse_one_##__tname(it, pa)) goto fail;

#endif // __BIPA_H__
