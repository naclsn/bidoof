#ifndef __BIPA_H__
#define __BIPA_H__

// TODO: add bitset/flags, backed by a num

/// public macros:
///  - bipa_struct(typename, fields count, ... fields type+name pairs)
///  - bipa_union(typename, kinds count, ... kinds type+(tag type, tag value, name) pairs)
///  - bipa_array(typename, item type)
///
/// types for fields can be:
///  - integral types: (u8, uNNle or uNNbe up to NN=64 bits little/big endian)
///  - string types: `(cstr, sentinel byte)` or `(lstr, length expression)`
///  - struct types: `(struct, typename)` where typename is a `bipa_struct(typename, ...)`
///  - union types: `(union, typename)` where ...
///  - array types: `(array, typename, while)` where ...
///
/// in an `(array, typename, while)`, the while is an expression with:
///  - `self` the encompassing struct
///  - `k` the iteration number
///  - `it` the array itself (so `it->ptr[k-1]` the last parsed item, `it->ptr[k]` is not parsed yet)
///
/// using one of the public macros will generate the associated functions:
///  - void bipa_dump_typename(cref self, FILE ref strm, int depth)
///  - bool bipa_build_typename(cref self, buf ref res)
///  - bool bipa_parse_typename(ref self, buf cref src, sz ref at)
///  - void bipa_free_typename(cref self)
///
/// short example:
/// ```c
/// bipa_struct(bidoof, 3
///     , (cstr, '\0'), name
///     , (u32le,), age // ',' after 'le' to make std=c99 happy
///     )
///
/// bipa_union(maybe_bidoof, 2
///     , (struct, bidoof), (u8, '*', some)
///     , (void,), (void, 0, none) // ',' same idea
///     )
///
/// // will match a buffer starting with a '*' character
/// // and parse a `bidoof` right after, that is a '\0'-
/// // terminated str followed by a 32bits little endian
/// bipa_parse_maybe_bidoof(&res, &buf);
/// if (maybe_bidoof_tag_some == res.tag)
///     res.val.some;
/// ```
///
/// also:
///  - define BIPA_DECLONLY to only have declaration, no implementation
///  - define BIPA_HIDUMP to use ANSI-escape coloration with bipa_dump_xyz
///
/// ---
///
/// internal types are defined with a set of macro:
///  - _typename_xyz(...)
///  - _dump_xyz(...)
///  - _build_xyz(...)
///  - _parse_xyz(...)
///  - _free_xyz(...)
/// the arguments to the macro are the ones given when using it in a type
/// eg `(array, typename, while)` the `typename, while`
///
/// a `_build` or `_parse` macro is a sequence of statements with:
///  - `it` is the source (resp. destination) pointer
///  - writing a byte: `res->ptr[res->len++] = *it`
///  - reading a byte: `*it = src->ptr[(*at)++]`

// uNN/sz typedefs, buf typedef and dyarr_ macros, ref/cref macros
#include "../base.h"

#ifdef BIPA_DECLONLY
#define _declonly(...) ;
#else
#define _declonly(...) __VA_ARGS__
#endif

#ifdef BIPA_HIDUMP
#define _hidump_kw(__c) "\x1b[34m" __c "\x1b[m"
#define _hidump_ty(__c) "\x1b[32m" __c "\x1b[m"
#define _hidump_nb(__c) "\x1b[33m" __c "\x1b[m"
#define _hidump_st(__c) "\x1b[36m" __c "\x1b[m"
#define _hidump_ex(__c) "\x1b[35m" __c "\x1b[m"
#define _hidump_id(__c) __c
#else
#define _hidump_kw(__c) __c
#define _hidump_ty(__c) __c
#define _hidump_nb(__c) __c
#define _hidump_st(__c) __c
#define _hidump_ex(__c) __c
#define _hidump_id(__c) __c
#endif

static void bipa_xxd(FILE ref strm, u8 cref ptr, sz const len, int depth) _declonly({
    if (0 == len) return;
    sz const top = (len-1)/16+1;
    for (sz j = 0; j < top; j++) {
        fprintf(strm, "\n%*.s", (depth+2)*2-1, "");
        for (sz i = 0; i < 16; i++) {
            sz const k = i+16*j;
            if (len <= k) {
                if (len < 16) break;
                fprintf(strm, "   ");
            } else fprintf(strm, " " _hidump_st("%02X"), ptr[k]);
        }
        fprintf(strm, "    ");
        for (sz i = 0; i < 16; i++) {
            sz const k = i+16*j;
            if (len <= k) break;
            char const it = ptr[i+16*j];
            fprintf(strm, _hidump_st("%c"), ' ' <= it && it <= '~' ? it : '.');
        }
    }
})

#define _STR(__ln) #__ln
#define _XSTR(__ln) _HERE_STR(__ln)
#define _JOIN(__l, __r) __l##__r
#define _XJOIN(__l, __r) _JOIN(__l, __r)
#define _CALL(__macro, ...) __macro(__VA_ARGS__)

#define  _FOR_TYNM_1(__n, __macro, __inv, __ty, __nm)       __macro((__n- 1), __n, __inv, __ty, __nm)
#define  _FOR_TYNM_2(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n- 2), __n, __inv, __ty, __nm)  _FOR_TYNM_1(__n, __macro, __inv, __VA_ARGS__)
#define  _FOR_TYNM_3(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n- 3), __n, __inv, __ty, __nm)  _FOR_TYNM_2(__n, __macro, __inv, __VA_ARGS__)
#define  _FOR_TYNM_4(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n- 4), __n, __inv, __ty, __nm)  _FOR_TYNM_3(__n, __macro, __inv, __VA_ARGS__)
#define  _FOR_TYNM_5(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n- 5), __n, __inv, __ty, __nm)  _FOR_TYNM_4(__n, __macro, __inv, __VA_ARGS__)
#define  _FOR_TYNM_6(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n- 6), __n, __inv, __ty, __nm)  _FOR_TYNM_5(__n, __macro, __inv, __VA_ARGS__)
#define  _FOR_TYNM_7(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n- 7), __n, __inv, __ty, __nm)  _FOR_TYNM_6(__n, __macro, __inv, __VA_ARGS__)
#define  _FOR_TYNM_8(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n- 8), __n, __inv, __ty, __nm)  _FOR_TYNM_7(__n, __macro, __inv, __VA_ARGS__)
#define  _FOR_TYNM_9(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n- 9), __n, __inv, __ty, __nm)  _FOR_TYNM_8(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_10(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-10), __n, __inv, __ty, __nm)  _FOR_TYNM_9(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_11(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-11), __n, __inv, __ty, __nm) _FOR_TYNM_10(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_12(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-12), __n, __inv, __ty, __nm) _FOR_TYNM_11(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_13(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-13), __n, __inv, __ty, __nm) _FOR_TYNM_12(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_14(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-14), __n, __inv, __ty, __nm) _FOR_TYNM_13(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_15(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-15), __n, __inv, __ty, __nm) _FOR_TYNM_14(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_16(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-16), __n, __inv, __ty, __nm) _FOR_TYNM_15(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_17(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-17), __n, __inv, __ty, __nm) _FOR_TYNM_16(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_18(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-18), __n, __inv, __ty, __nm) _FOR_TYNM_17(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_19(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-19), __n, __inv, __ty, __nm) _FOR_TYNM_18(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_20(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-20), __n, __inv, __ty, __nm) _FOR_TYNM_19(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_21(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-21), __n, __inv, __ty, __nm) _FOR_TYNM_20(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_22(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-22), __n, __inv, __ty, __nm) _FOR_TYNM_21(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_23(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-23), __n, __inv, __ty, __nm) _FOR_TYNM_22(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_24(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-24), __n, __inv, __ty, __nm) _FOR_TYNM_23(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_25(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-25), __n, __inv, __ty, __nm) _FOR_TYNM_24(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_26(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-26), __n, __inv, __ty, __nm) _FOR_TYNM_25(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_27(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-27), __n, __inv, __ty, __nm) _FOR_TYNM_26(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_28(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-28), __n, __inv, __ty, __nm) _FOR_TYNM_27(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM_29(__n, __macro, __inv, __ty, __nm, ...)  __macro((__n-29), __n, __inv, __ty, __nm) _FOR_TYNM_28(__n, __macro, __inv, __VA_ARGS__)
#define _FOR_TYNM(__n, __macro, __inv, ...)  _FOR_TYNM_##__n(__n, __macro, __inv, __VA_ARGS__)

#define _typename(__ty, ...) _CALL(_typename_##__ty, __VA_ARGS__)
#define _dump(__ty, ...)     _CALL(_dump_##__ty, __VA_ARGS__)
#define _build(__ty, ...)    _CALL(_build_##__ty, __VA_ARGS__)
#define _parse(__ty, ...)    _CALL(_parse_##__ty, __VA_ARGS__)
#define _free(__ty, ...)     _CALL(_free_##__ty, __VA_ARGS__)

#define _typename_void() bool
#define _dump_void()  (void)it; fprintf(strm, _hidump_kw("void"));
#define _build_void() (void)it;
#define _parse_void() (void)it;
#define _free_void()  (void)it;

#define _typename_u8() u8
#define _dump_u8()  fprintf(strm, _hidump_nb("%hhu") _hidump_ex("u8"), *it);
#define _build_u8() { u8* p = dyarr_push(res);  \
                    if (!p) goto fail;  \
                    *p = *it; }
#define _parse_u8() if (src->len == (*at)) goto fail;  \
                    *it = src->ptr[(*at)++];
#define _free_u8()  (void)it;

#define _typename_u16le() u16
#define _typename_u32le() u32
#define _typename_u64le() u64
#define _dump_u16le()   fprintf(strm, _hidump_nb("%hu") _hidump_ex("u16le"), *it);
#define _dump_u32le()   fprintf(strm, _hidump_nb("%u" ) _hidump_ex("u32le"), *it);
#define _dump_u64le()   fprintf(strm, _hidump_nb("%lu") _hidump_ex("u64le"), *it);
#define _build_u16le()  if (res->cap <= res->len+1 &&                    \
                            !dyarr_resize(res,                           \
                                res->cap ? res->cap*2 : 16)) goto fail;  \
                        res->ptr[res->len++] = *it&0xff;                 \
                        res->ptr[res->len++] = (*it>>8)&0xff;
#define _build_u32le()  if (res->cap <= res->len+3 &&                    \
                            !dyarr_resize(res,                           \
                                res->cap ? res->cap*2 : 16)) goto fail;  \
                        res->ptr[res->len++] = *it&0xff;                 \
                        res->ptr[res->len++] = (*it>>8)&0xff;            \
                        res->ptr[res->len++] = (*it>>16)&0xff;           \
                        res->ptr[res->len++] = (*it>>24)&0xff;
#define _build_u64le()  if (res->cap <= res->len+7 &&                    \
                            !dyarr_resize(res,                           \
                                res->cap ? res->cap*2 : 16)) goto fail;  \
                        res->ptr[res->len++] = *it&0xff;                 \
                        res->ptr[res->len++] = (*it>>8)&0xff;            \
                        res->ptr[res->len++] = (*it>>16)&0xff;           \
                        res->ptr[res->len++] = (*it>>24)&0xff;           \
                        res->ptr[res->len++] = (*it>>32)&0xff;           \
                        res->ptr[res->len++] = (*it>>40)&0xff;           \
                        res->ptr[res->len++] = (*it>>48)&0xff;           \
                        res->ptr[res->len++] = (*it>>56)&0xff;
#define _parse_u16le()  if (src->len <= (*at)+1) goto fail;              \
                        *it = (u16)src->ptr[(*at)]                       \
                            | (u16)src->ptr[(*at)+1]<<8;                 \
                        (*at)+= 2;
#define _parse_u32le()  if (src->len <= (*at)+3) goto fail;              \
                        *it = (u32)src->ptr[(*at)]                       \
                            | (u32)src->ptr[(*at)+1]<<8                  \
                            | (u32)src->ptr[(*at)+2]<<16                 \
                            | (u32)src->ptr[(*at)+3]<<24;                \
                        (*at)+= 4;
#define _parse_u64le()  if (src->len <= (*at)+7) goto fail;              \
                        *it = (u64)src->ptr[(*at)]                       \
                            | (u64)src->ptr[(*at)+1]<<8                  \
                            | (u64)src->ptr[(*at)+2]<<16                 \
                            | (u64)src->ptr[(*at)+3]<<24                 \
                            | (u64)src->ptr[(*at)+4]<<32                 \
                            | (u64)src->ptr[(*at)+5]<<40                 \
                            | (u64)src->ptr[(*at)+6]<<48                 \
                            | (u64)src->ptr[(*at)+7]<<56;                \
                        (*at)+= 8;
#define _free_u16le() (void)it;
#define _free_u32le() (void)it;
#define _free_u64le() (void)it;

#define _typename_u16be() u16
#define _typename_u32be() u32
#define _typename_u64be() u64
#define _dump_u16be()   fprintf(strm, _hidump_nb("%hu") _hidump_ex("u16be"), *it);
#define _dump_u32be()   fprintf(strm, _hidump_nb("%u" ) _hidump_ex("u32be"), *it);
#define _dump_u64be()   fprintf(strm, _hidump_nb("%lu") _hidump_ex("u64be"), *it);
#define _build_u16be()  if (res->cap <= res->len+1 &&                    \
                            !dyarr_resize(res,                           \
                                res->cap ? res->cap*2 : 16)) goto fail;  \
                        res->ptr[res->len++] = (*it>>8)&0xff;            \
                        res->ptr[res->len++] = *it&0xff;
#define _build_u32be()  if (res->cap <= res->len+3 &&                    \
                            !dyarr_resize(res,                           \
                                res->cap ? res->cap*2 : 16)) goto fail;  \
                        res->ptr[res->len++] = (*it>>24)&0xff;           \
                        res->ptr[res->len++] = (*it>>16)&0xff;           \
                        res->ptr[res->len++] = (*it>>8)&0xff;            \
                        res->ptr[res->len++] = *it&0xff;
#define _build_u64be()  if (res->cap <= res->len+7 &&                    \
                            !dyarr_resize(res,                           \
                                res->cap ? res->cap*2 : 16)) goto fail;  \
                        res->ptr[res->len++] = (*it>>56)&0xff;           \
                        res->ptr[res->len++] = (*it>>48)&0xff;           \
                        res->ptr[res->len++] = (*it>>40)&0xff;           \
                        res->ptr[res->len++] = (*it>>32)&0xff;           \
                        res->ptr[res->len++] = (*it>>24)&0xff;           \
                        res->ptr[res->len++] = (*it>>16)&0xff;           \
                        res->ptr[res->len++] = (*it>>8)&0xff;            \
                        res->ptr[res->len++] = *it&0xff;
#define _parse_u16be()  if (src->len <= (*at)+1) goto fail;              \
                        *it = (u16)src->ptr[(*at)+1]                     \
                            | (u16)src->ptr[(*at)]<<8;                   \
                        (*at)+= 2;
#define _parse_u32be()  if (src->len <= (*at)+3) goto fail;              \
                        *it = (u32)src->ptr[(*at)+3]                     \
                            | (u32)src->ptr[(*at)+2]<<8                  \
                            | (u32)src->ptr[(*at)+1]<<16                 \
                            | (u32)src->ptr[(*at)]<<24;                  \
                        (*at)+= 4;
#define _parse_u64be()  if (src->len <= (*at)+7) goto fail;              \
                        *it = (u64)src->ptr[(*at)+7]                     \
                            | (u64)src->ptr[(*at)+6]<<8                  \
                            | (u64)src->ptr[(*at)+5]<<16                 \
                            | (u64)src->ptr[(*at)+4]<<24                 \
                            | (u64)src->ptr[(*at)+3]<<32                 \
                            | (u64)src->ptr[(*at)+2]<<40                 \
                            | (u64)src->ptr[(*at)+1]<<48                 \
                            | (u64)src->ptr[(*at)]<<56;                  \
                        (*at)+= 8;
#define _free_u16be() (void)it;
#define _free_u32be() (void)it;
#define _free_u64be() (void)it;

#define _typename_cstr(__sentinel) u8*
#define _dump_cstr(__sentinel) {                                         \
        u8 s = (__sentinel);                                             \
        sz len = 0;                                                      \
        for (sz k = 0; s != (*it)[k]; k++) len++;                        \
        fprintf(strm, _hidump_kw("cstr")                                 \
                "(" _hidump_nb("%zu") _hidump_ex("+1") ")", len);        \
        bipa_xxd(strm, *it, len, depth);                                 \
        if (!(len & 0xf)) fprintf(strm, "\n%*.s", (depth+2)*2-1, "");    \
        fprintf(strm, _hidump_ex(" %02X"), s);                           \
    }
#define _build_cstr(__sentinel) {                            \
        u8 s = (__sentinel);                                 \
        sz len = (u8*)strchr((char*)*it, s) - *it;           \
        while (res->cap <= res->len+len+1)                   \
            if (!dyarr_resize(res,                           \
                    res->cap ? res->cap*2 : 16)) goto fail;  \
        memcpy(res->ptr+res->len, *it, len);                 \
        res->len+= len;                                      \
        res->ptr[res->len++] = s;                            \
    }
#define _parse_cstr(__sentinel) {                                \
        u8* from = src->ptr+(*at);                               \
        u8* found = memchr(from, (__sentinel), src->len-(*at));  \
        if (!found) goto fail;                                   \
        sz len = found-from;                                     \
        *it = malloc(len+1);                                     \
        if (!*it) goto fail;                                     \
        memcpy(*it, from, len+1);                                \
        (*at)+= len+1;                                           \
    }
#define _free_cstr(__sentinel) free(*it);

#define _typename_lstr(__length) u8*
#define _dump_lstr(__length) {                                             \
        sz len = (__length);                                               \
        fprintf(strm, _hidump_kw("lstr") "(" _hidump_nb("%zu") ")", len);  \
        bipa_xxd(strm, *it, len, depth);                                   \
    }
#define _build_lstr(__length) {                              \
        sz len = (__length);                                 \
        while (res->cap <= res->len+len+1)                   \
            if (!dyarr_resize(res,                           \
                    res->cap ? res->cap*2 : 16)) goto fail;  \
        memcpy(res->ptr+res->len, *it, len);                 \
        res->len+= len;                                      \
    }
#define _parse_lstr(__length) {               \
        u8* from = src->ptr+(*at);            \
        sz len = (__length);                  \
        if (src->len < (*at)+len) goto fail;  \
        *it = malloc(len);                    \
        if (!*it) goto fail;                  \
        memcpy(*it, from, len);               \
        (*at)+= len;                          \
    }
#define _free_lstr(__sentinel) free(*it);

#define _struct_fields_typename_one(__k, __n, __inv, __ty, __nm) _typename __ty __nm;

#define _struct_fields_dump_one(__k, __n, __inv, __ty, __nm) {  \
        fprintf(strm, "." _hidump_id(#__nm) "= ");              \
        _typename __ty const* const it = &self->__nm;           \
        _dump __ty                                              \
        fprintf(strm, ",\n%*.s", (depth+(__k+1!=__n))*2, "");   \
    }

#define _struct_fields_build_one(__k, __n, __inv, __ty, __nm) {  \
        _typename __ty const* const it = &self->__nm;            \
        _build __ty                                              \
    }

#define _struct_fields_parse_one(__k, __n, __inv, __ty, __nm) {  \
        _typename __ty* const it = &self->__nm;                  \
        _parse __ty                                              \
    }

#define _struct_fields_free_one(__k, __n, __inv, __ty, __nm) {  \
        _typename __ty const* const it = &self->__nm;           \
        _free __ty                                              \
    }

#define bipa_struct(__tname, __n_fields, ...)                                                          \
    struct __tname {                                                                                   \
        _FOR_TYNM(__n_fields, _struct_fields_typename_one, 0, __VA_ARGS__)                             \
    };                                                                                                 \
    void bipa_dump_##__tname(struct __tname cref self, FILE* const strm, int const depth) _declonly({  \
        (void)depth;                                                                                   \
        fprintf(strm, _hidump_kw("struct") " " _hidump_ty(#__tname) " {\n%*.s", (depth+1)*2, "");      \
        _FOR_TYNM(__n_fields, _struct_fields_dump_one, 0, __VA_ARGS__)                                 \
        fprintf(strm, "}");                                                                            \
    })                                                                                                 \
    bool bipa_build_##__tname(struct __tname cref self, buf ref res) _declonly({                       \
        _FOR_TYNM(__n_fields, _struct_fields_build_one, 0, __VA_ARGS__)                                \
        return true;                                                                                   \
    fail: return false;                                                                                \
    })                                                                                                 \
    bool bipa_parse_##__tname(struct __tname ref self, buf cref src, sz ref at) _declonly({            \
        sz at_before = (*at);                                                                          \
        _FOR_TYNM(__n_fields, _struct_fields_parse_one, 0, __VA_ARGS__)                                \
        return true;                                                                                   \
    fail:                                                                                              \
        (*at) = at_before;                                                                             \
        return false;                                                                                  \
    })                                                                                                 \
    void bipa_free_##__tname(struct __tname cref self) _declonly({                                     \
        _FOR_TYNM(__n_fields, _struct_fields_free_one, 0, __VA_ARGS__)                                 \
    })
#define _typename_struct(__tname) struct __tname
#define _dump_struct(__tname)   bipa_dump_##__tname(it, strm, depth+1);
#define _build_struct(__tname)  if (!bipa_build_##__tname(it, res)) goto fail;
#define _parse_struct(__tname)  if (!bipa_parse_##__tname(it, src, at)) goto fail;
#define _free_struct(__tname)   bipa_free_##__tname(it);

#define _union_getvar_name(__tty, __tva, __nm)     __nm
#define _union_getvar_tagtype(__tty, __tva, __nm)  __tty
#define _union_getvar_tagvalue(__tty, __tva, __nm) __tva

#define _union_kinds_typename_one(__k, __n, __inv, __ty, __nm) _typename __ty _union_getvar_name __nm;
#define _union_kinds_tagname_one(__k, __n, __tname, __ty, __nm) _XJOIN(__tname##_tag_, _union_getvar_name __nm) = __k,

#define _union_kinds_dump_one(__k, __n, __tname, __ty, __nm)     \
    case _XJOIN(__tname##_tag_, _union_getvar_name __nm): {      \
        fprintf(strm, "|");                                      \
        {                                                        \
            _XJOIN(_typename_, _union_getvar_tagtype __nm)()     \
                const _it = _union_getvar_tagvalue __nm,         \
                * const it = &_it;                               \
            _XJOIN(_dump_, _union_getvar_tagtype __nm)()         \
        }                                                        \
        fprintf(strm, "." _XSTR(_union_getvar_name __nm) "| ");  \
        {                                                        \
            _typename __ty const* const it =                     \
                &self->val._union_getvar_name __nm;              \
            _dump __ty                                           \
        }                                                        \
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

#define _union_kinds_free_one(__k, __n, __tname, __ty, __nm)  \
    case _XJOIN(__tname##_tag_, _union_getvar_name __nm): {   \
        _typename __ty const* const it =                      \
            &self->val._union_getvar_name __nm;               \
        _free __ty                                            \
    } break;

#define bipa_union(__tname, __n_kinds, ...)                                                            \
    struct __tname {                                                                                   \
        union {                                                                                        \
            _FOR_TYNM(__n_kinds, _union_kinds_typename_one, 0, __VA_ARGS__)                            \
        } val;                                                                                         \
        enum {                                                                                         \
            _FOR_TYNM(__n_kinds, _union_kinds_tagname_one, __tname, __VA_ARGS__)                       \
        } tag;                                                                                         \
    };                                                                                                 \
    void bipa_dump_##__tname(struct __tname cref self, FILE* const strm, int const depth) _declonly({  \
        (void)depth;                                                                                   \
        switch (self->tag) {                                                                           \
            _FOR_TYNM(__n_kinds, _union_kinds_dump_one, __tname, __VA_ARGS__)                          \
            default: fprintf(strm, "|erroneous|");                                                     \
        }                                                                                              \
    })                                                                                                 \
    bool bipa_build_##__tname(struct __tname cref self, buf ref res) _declonly({                       \
        switch (self->tag) {                                                                           \
            _FOR_TYNM(__n_kinds, _union_kinds_build_one, __tname, __VA_ARGS__)                         \
            default: goto fail;                                                                        \
        }                                                                                              \
        return true;                                                                                   \
    fail: return false;                                                                                \
    })                                                                                                 \
    bool bipa_parse_##__tname(struct __tname ref self, buf cref src, sz ref at) _declonly({            \
        sz at_before = (*at);                                                                          \
        for (sz _k_kinds = 0; _k_kinds < __n_kinds; _k_kinds++) {                                      \
            switch (_k_kinds) {                                                                        \
                _FOR_TYNM(__n_kinds, _union_kinds_parse_one, __tname, __VA_ARGS__)                     \
            }                                                                                          \
            return true;                                                                               \
        fail: (*at) = at_before;                                                                       \
        }                                                                                              \
        return false;                                                                                  \
    })                                                                                                 \
    void bipa_free_##__tname(struct __tname cref self) _declonly({                                     \
        switch (self->tag) {                                                                           \
            _FOR_TYNM(__n_kinds, _union_kinds_free_one, __tname, __VA_ARGS__)                          \
        }                                                                                              \
    })
#define _typename_union(__tname) struct __tname
#define _dump_union(__tname)    bipa_dump_##__tname(it, strm, depth+1);
#define _build_union(__tname)   if (!bipa_build_##__tname(it, res)) goto fail;
#define _parse_union(__tname)   if (!bipa_parse_##__tname(it, src, at)) goto fail;
#define _free_union(__tname)    bipa_free_##__tname(it);

#define bipa_array(__tname, __of)                                                                      \
    struct __tname {                                                                                   \
        _typename __of* ptr;                                                                           \
        sz len, cap;                                                                                   \
    };                                                                                                 \
    void bipa_dump_##__tname(struct __tname cref self, FILE* const strm, int const depth) _declonly({  \
        if (!self->len) {                                                                              \
            fprintf(strm, _hidump_kw("array") " " _hidump_ty(#__tname) " []");                         \
            return;                                                                                    \
        }                                                                                              \
        fprintf(strm, _hidump_kw("array") " " _hidump_ty(#__tname) " [\n%*.s",                         \
                (depth+1)*2, "");                                                                      \
        for (sz k = 0; k < self->len; k++) {                                                           \
            _typename __of const* const it = self->ptr + k;                                            \
            _dump __of                                                                                 \
            fprintf(strm, ",\n%*.s", (depth+(k+1!=self->len))*2, "");                                  \
        }                                                                                              \
        fprintf(strm, "]");                                                                            \
    })                                                                                                 \
    bool bipa_build_##__tname(struct __tname cref self, buf ref res) _declonly({                       \
        for (sz k = 0; k < self->len; k++) {                                                           \
            _typename __of const* const it = self->ptr + k;                                            \
            _build __of                                                                                \
        }                                                                                              \
        return true;                                                                                   \
    fail: return false;                                                                                \
    })                                                                                                 \
    bool bipa_parse_one_##__tname(struct __tname ref self, buf cref src, sz ref at) _declonly({        \
        sz at_before = (*at);                                                                          \
        _typename __of* const it = dyarr_push(self);                                                   \
        if (!it) return false;                                                                         \
        _parse __of                                                                                    \
        return true;                                                                                   \
    fail:                                                                                              \
        (*at) = at_before;                                                                             \
        return false;                                                                                  \
    })                                                                                                 \
    void bipa_free_##__tname(struct __tname cref self) _declonly({                                     \
        for (sz k = 0; k < self->len; k++) {                                                           \
            _typename __of const* const it = self->ptr + k;                                            \
            _free __of                                                                                 \
        }                                                                                              \
        free(self->ptr);                                                                               \
    })
#define _typename_array(__tname, __while) struct __tname
#define _dump_array(__tname, __while)   bipa_dump_##__tname(it, strm, depth+1);
#define _build_array(__tname, __while)  if (!bipa_build_##__tname(it, res)) goto fail;
#define _parse_array(__tname, __while)  it->ptr = NULL;  \
                                        for (sz k = it->len = it->cap = 0; __while; k++)  \
                                            if (!bipa_parse_one_##__tname(it, src, at)) goto fail;
#define _free_array(__tname, __while)   bipa_free_##__tname(it);

#endif // __BIPA_H__
