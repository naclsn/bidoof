#ifndef __PARAS_H__
#define __PARAS_H__

// uNN/sz typedefs, buf typedef and dyarr_ macros, ref/cref macros
#include "../base.h"

#ifdef PARAS_DECLONLY
#define _declonly(...) ;
#else
#define _declonly(...) __VA_ARGS__
#endif

#define __rem_par(...) __VA_ARGS__
#define __first_arg(h, ...) h

union paras_generic { char const* s; int i; unsigned u; };
static bool _paras_scan(buf cref src, sz ref at, char cref f, sz const argc, union paras_generic argv[argc]) _declonly({
    printf("NYI: scan <<%s>>, %zu args\n", f, argc);
    return false;
})

#define _paras_make_statics(__name, __codes, __masks, __format, __encode)  \
    static u8 const codes[] = {__rem_par __codes};                         \
    static u8 const masks[countof(codes)] = {__rem_par __masks};

#define _paras_make_disasm(__name, __codes, __masks, __format, __encode)  \
    unsigned _k;                                                          \
    for (_k = 0; _k < countof(codes); _k++)                               \
        if ((bytes[_k] & masks[_k]) != codes[_k]) break;                  \
    if (countof(codes) == _k) {                                           \
        printf("%07zx0:   ", at);                                         \
        for (unsigned k = 0; k < 5; k++)                                  \
            if (k < _k) printf("%02X ", bytes[k]);                        \
            else printf("   ");                                           \
        printf("  " #__name " \t");                                       \
        printf __format;                                                  \
        printf("\n");                                                     \
        at+= _k;                                                          \
        continue;                                                         \
    }

#define _paras_make_asmbl(__name, __code, __masks, __format, __encode)                    \
    if (!strcmp(#__name, word)) {                                                         \
        union paras_generic* args = NULL;                                                 \
        union paras_generic _args[countof((u8[]){__rem_par __encode})];                   \
        if (_paras_scan(src, &at, __first_arg __format, countof(_args), args = _args)) {  \
            u8 const _bytes[countof(codes)] = {__rem_par __encode};                       \
            (void)_bytes;                                                                 \
        }                                                                                 \
    }

#define paras_make_instr(...) {                           \
        _paras_make_statics(__VA_ARGS__)                  \
        if (_disasm) { _paras_make_disasm(__VA_ARGS__) }  \
        else { _paras_make_asmbl(__VA_ARGS__) }           \
    }

#define paras_make_instrset(__name, ...)                                 \
    sz paras_disasm_##__name(buf cref src, buf ref res) _declonly({      \
        static bool const _disasm = true;                                \
        static char const* const word;                                   \
        sz at = 0;                                                       \
        while (at < src->len) {                                          \
            u8 const* const bytes = src->ptr+at;                         \
            __VA_ARGS__                                                  \
            break;                                                       \
        }                                                                \
        return at;                                                       \
    })                                                                   \
                                                                         \
    sz paras_asmbl_##__name(buf cref src, buf ref r) _declonly({         \
        static bool const _disasm = false;                               \
        static u8 const* const bytes;                                    \
        sz at = 0;                                                       \
        while (at < src->len) {                                          \
            while (strchr(" \t", src->ptr[at]) && at < src->len) at++;   \
            char const* const word = (char*)src->ptr+at;                 \
            while (!strchr(" \t", src->ptr[at]) && at < src->len) at++;  \
            if (at < src->len && ';' != *word) { __VA_ARGS__ }           \
            while ('\n' != src->ptr[at] && at < src->len) at++;          \
        }                                                                \
        return at;                                                       \
    })

#endif // __PARAS_H__
