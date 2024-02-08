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
#define __first_arg_(h, ...) h
#define __first_arg(...) __first_arg_(__VA_ARGS__, _)

union paras_generic { char const* s; int i; unsigned u; };
static bool _paras_scan(buf cref src, sz ref at, char cref f, sz const argc, union paras_generic argv[argc]) {
    sz eol = *at;
    while ('\n' != src->ptr[eol] && eol < src->len) eol++;

    sz i = *at, j = 0, k = 0;
    while (f[j] && i < eol && ';' != src->ptr[i] && k < argc) switch (f[j++]) {
        case ' ':
            while (strchr(" \t", src->ptr[i]) && i < eol) i++;
            break;

        case '%': switch (f[j++]) {
                case 's':
                    argv[k++].s = (char*)src->ptr+i;
                    while (!strchr(" \t;,][", src->ptr[i]) && i < eol) i++;
                    break;

                case 'u':
                    if ('-' == src->ptr[i]) return false;
                    // fallthrough
                case 'i': {
                    bool const minus = '-' == src->ptr[i];
                    if (minus || '+' == src->ptr[i]) i++;
                    unsigned shft = 0;
                    char const* dgts = "0123456789";
                    if ('0' == src->ptr[i]) switch (src->ptr[i++]) {
                        case 'b': i++; shft = 1; dgts = "01";               break;
                        case 'o': i++; shft = 3; dgts = "01234567";         break;
                        case 'x': i++; shft = 4; dgts = "0123456789abcdef"; break;
                    }
                    argv[k].i = 0;
                    u8 const* c = src->ptr+i;
                    for (char const* v; c < src->ptr+eol && (v = strchr(dgts, *c|32)); c++)
                        argv[k].i = (!shft ? argv[k].i*10 : argv[k].i<<shft) + (v-dgts);
                    if (src->ptr+i == c) return false;
                    if (minus) argv[k].i*= -1;
                    k++;
                    i = c-src->ptr;
                } break;

                case '%': if ('%' != src->ptr[i++]) return false;
            } break;

        default: if (f[j-1] != src->ptr[i++]) return false;
    }

    *at = eol+1;
    return true;
}

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

#define _paras_make_asmbl(__name, __codes, __masks, __format, __encode)                   \
    if (!memcmp(#__name, word, strlen(#__name))) {                                        \
        union paras_generic* args = NULL;                                                 \
        union paras_generic _args[countof((u8[]){__rem_par __codes})];                    \
        if (_paras_scan(src, &at, __first_arg __format, countof(_args), args = _args)) {  \
            u8 const _bytes[countof(codes)] = {__rem_par __encode};                       \
            for (unsigned k = 0; k < countof(codes); k++)  \
                printf(" %02X", _bytes[k]);  \
            puts(" - " #__name);  \
            continue;                                                                     \
        }                                                                                 \
    }

#define paras_make_instr(...) {                           \
        _paras_make_statics(__VA_ARGS__)                  \
        if (_disasm) { _paras_make_disasm(__VA_ARGS__) }  \
        else { _paras_make_asmbl(__VA_ARGS__) }           \
    }

#define paras_make_instrset(__name, ...)                                   \
    sz paras_disasm_##__name(buf cref src, buf ref res) _declonly({        \
        (void)res;  \
        static bool const _disasm = true;                                  \
        static char const* const word;                                     \
        sz at = 0;                                                         \
        while (at < src->len) {                                            \
            u8 const* const bytes = src->ptr+at;                           \
            __VA_ARGS__                                                    \
            break;                                                         \
        }                                                                  \
        return at;                                                         \
    })                                                                     \
                                                                           \
    sz paras_asmbl_##__name(buf cref src, buf ref res) _declonly({         \
        (void)res;  \
        static bool const _disasm = false;                                 \
        static u8 const* const bytes;                                      \
        sz at = 0;                                                         \
        while (at < src->len) {                                            \
            while (strchr(" \t\n", src->ptr[at]) && at < src->len) at++;   \
            char const* const word = (char*)src->ptr+at;                   \
            while (!strchr(" \t\n", src->ptr[at]) && at < src->len) at++;  \
            while (strchr(" \t", src->ptr[at]) && at < src->len) at++;     \
            if (at < src->len && ';' != *word) {                           \
                __VA_ARGS__                                                \
                exitf("unknown instruction mnemonic: '%.*s'",              \
                    (int)(src->ptr+at-(u8*)word), word);                   \
            }                                                              \
            while ('\n' != src->ptr[at] && at < src->len) at++;            \
        }                                                                  \
        return at;                                                         \
    })

#endif // __PARAS_H__
