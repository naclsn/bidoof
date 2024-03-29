// TODO: doc

/// (here)
///
/// formats,
/// are (intentionally) supported:
///  - %s, args[k].s will point to the beginning of the matched text (not 0-terminated)
///        (end delim are " \t\n;,]+-", if starts with '[', ']' is accepted)
///  - %i or %+i, args[k].i will be the parsed integer (accepts leading +/- and 0b/0o/0x)
///  - %u, %x or %#x, same but fails if number starts with a '-'

#undef _paras_declonly
#ifdef PARAS_DECLONLY
#define _paras_declonly(...) ;
#else
#define _paras_declonly(...) __VA_ARGS__
#endif

#ifndef PARAS_NOTIFY
#define PARAS_NOTIFY(...) ((void)0)
#endif

// uNN/sz typedefs, buf typedef and dyarr_ macros, ref/cref macros
#include "../base.h"

union paras_generic { char const* s; int i; unsigned u; };
static bool _paras_scan(buf cref src, sz ref at, unsigned const _ln, char cref f, union paras_generic* args) _paras_declonly({
    sz i = *at, j = 0, k = 0;
    while (f[j] && i < src->len && '\n' != src->ptr[i] && ';' != src->ptr[i]) switch (f[j++]) {
        case ' ':
            while (i < src->len && strchr(" \t", src->ptr[i])) i++;
            break;

        case '%': switch ('#' == f[j] || '+' == f[j] ? j++, f[j++] : f[j++]) {
            case 's':
                args[k++].s = (char*)src->ptr+i;
                for (char const* const endchs = '[' == src->ptr[i]
                        ? " \t\n;,+-"
                        : " \t\n;,]+-";
                        i < src->len && !strchr(endchs, src->ptr[i]); i++);
                break;

            case 'u':
            case 'x':
                if ('-' == src->ptr[i]) {
                    PARAS_NOTIFY("expected unsigned (%%%c in format) but got '-' at offset %zu (line %u)", f[j-1], i, _ln);
                    return false;
                }
                // fallthrough
            case 'i': {
                bool const minus = '-' == src->ptr[i];
                if (minus || '+' == src->ptr[i]) i++;
                unsigned shft = 0;
                char const* dgts = "0123456789";
                if ('0' == src->ptr[i]) switch (src->ptr[++i]|32) {
                    case 'b': i++; shft = 1; dgts = "01";               break;
                    case 'o': i++; shft = 3; dgts = "01234567";         break;
                    case 'x': i++; shft = 4; dgts = "0123456789abcdef"; break;
                }
                args[k].i = 0;
                char const* v = strchr(dgts, src->ptr[i]|32);
                if (!v) {
                    PARAS_NOTIFY("expected digit in \"%s\" (%%%c in format) but got '%c' at offset %zu (line %u)", dgts, f[j-1], src->ptr[i-1], i-1, _ln);
                    return false;
                }
                do args[k].i = (!shft ? args[k].i*10 : args[k].i<<shft) + (v-dgts);
                while (++i < src->len && (v = strchr(dgts, src->ptr[i]|32)));
                if (minus) args[k].i*= -1;
                k++;
            } break;

            case '%': if ('%' != src->ptr[i++]) {
                PARAS_NOTIFY("expected literal '%%' but got '%c' at offset %zu (line %u)", src->ptr[i-1], i-1, _ln);
                return false;
            }
        } break;

        default: if (f[j-1] != src->ptr[i++]) {
            PARAS_NOTIFY("expected literal '%c' but got '%c' at offset %zu (line %u)", f[j-1], src->ptr[i-1], i-1, _ln);
            return false;
        }
    }

    if (f[j]) return false;
    *at = i;
    while (*at < src->len && '\n' != src->ptr[i]) {
        if (';' == src->ptr[*at]) {
            while (*at < src->len && '\n' != src->ptr[i]) i++;
            break;
        }
        if (!strchr(" \t", src->ptr[*at])) return false;
        ++*at;
    }

    return true;
})

#ifndef __PARAS_H__
#define __PARAS_H__

#define __rem_par(...) __VA_ARGS__
#define __first_arg_(h, ...) h
#define __first_arg(...) __first_arg_(__VA_ARGS__, _)

static inline int _paras_mark_invalid(bool* valid) { return *valid = false; }
#define paras_mark_invalid() _paras_mark_invalid(&valid)
#define _paras_sprintf_h(...) sprintf(_h, " " __VA_ARGS__)

#define _paras_make_statics(__name, __codes, __masks, __format, __encode)  \
    static u8 const masks[] = {__rem_par __masks};                         \
    static u8 const codes[countof(masks)] = {__rem_par __codes};

#define _paras_make_disasm(__name, __codes, __masks, __format, __encode)           \
    unsigned _k;                                                                   \
    for (_k = 0; _k < countof(codes) && _k < _ln; _k++)                            \
        if ((bytes[_k] & masks[_k]) != codes[_k]) break;                           \
    if (countof(codes) == _k) {                                                    \
        char _buf[256], *_h = _buf;                                                \
        _h+= sprintf(_h, #__name);                                                 \
        _h+= _paras_sprintf_h __format;                                            \
        _h+= sprintf(_h, "%*.s; %08zx:  ", (unsigned)(24-(_h-_buf)), "", loc+at);  \
        for (unsigned k = 0; k < _k; k++) _h+= sprintf(_h, " %02X", bytes[k]);     \
        _h+= sprintf(_h, "\n");                                                    \
        bufcat(res, (buf){.ptr= (u8*)_buf, .len= _h-_buf});                        \
        at+= _k;                                                                   \
        continue;                                                                  \
    }

#define _paras_make_asmbl(__name, __codes, __masks, __format, __encode)       \
    if (!memcmp(#__name, word, strlen(#__name)) &&                            \
            strchr(" \t\n;", word[strlen(#__name)])) {                        \
        union paras_generic args[4];                                          \
        sz _pat = at;                                                         \
        if (_paras_scan(src, &at, _ln, __first_arg __format, args)) {         \
            bool valid = true;                                                \
            u8 const _bytes[countof(codes)] = {__rem_par __encode};           \
            if (valid) {                                                      \
                bufcat(res, (buf){.ptr= (u8*)_bytes, .len= countof(codes)});  \
                continue;                                                     \
            } else at = _pat;                                                 \
        }                                                                     \
    }

#define paras_make_instr(...) {                           \
        _paras_make_statics(__VA_ARGS__)                  \
        if (_disasm) { _paras_make_disasm(__VA_ARGS__) }  \
        else { _paras_make_asmbl(__VA_ARGS__) }           \
    }

#define paras_make_instr_x(__name, __codes, __masks, __format_x, __encode_x) {                   \
        _paras_make_statics(__name, __codes, __masks, (), ())                                    \
        if (_disasm) {                                                                           \
            unsigned _k;                                                                         \
            for (_k = 0; _k < countof(codes) && _k < _ln; _k++)                                  \
                if ((bytes[_k] & masks[_k]) != codes[_k]) break;                                 \
            if (countof(codes) == _k) {                                                          \
                sz const _pat = at, _plen = res->len;                                            \
                bufcat(res, mkbuf(#__name " "));                                                 \
                if (__format_x(src, &at, res)) {                                                 \
                    char _buf[256], *_h = _buf;                                                  \
                    _h+= sprintf(_h, "%*.s; %08zx:  ", 24, "", loc+_pat);                        \
                    for (unsigned k = 0; k < at-_pat; k++) _h+= sprintf(_h, " %02X", bytes[k]);  \
                    _h+= sprintf(_h, "\n");                                                      \
                    bufcat(res, (buf){.ptr= (u8*)_buf, .len= _h-_buf});                          \
                    continue;                                                                    \
                }                                                                                \
                at = _pat;                                                                       \
                res->len = _plen;                                                                \
            }                                                                                    \
        } else {                                                                                 \
            if (!memcmp(#__name, word, strlen(#__name)) &&                                       \
                    strchr(" \t\n;", word[strlen(#__name)])) {                                   \
                buf line = {.ptr= src->ptr+at, .len= src->len-at};                               \
                sz _k = 0;                                                                       \
                while (_k < line.len && !strchr("\n;", line.ptr[_k])) _k++;                      \
                line.len = _k;                                                                   \
                while (1 < line.len && strchr(" \t", line.ptr[line.len-1])) line.len--;          \
                if (__encode_x(&line, res)) {                                                    \
                    at+= _k;                                                                     \
                    while (at < src->len && '\n' != src->ptr[at]) at++;                          \
                    continue;                                                                    \
                }                                                                                \
            }                                                                                    \
        }                                                                                        \
    }

#define paras_make_instrset(__name, ...)                                                 \
    sz paras_disasm_##__name(buf cref src, buf ref res, sz const loc) _paras_declonly({  \
        static bool const _disasm = true;                                                \
        static char const* const word;                                                   \
        unsigned _ln;                                                                    \
        sz at = 0;                                                                       \
        while (at < src->len) {                                                          \
            u8 const* const bytes = src->ptr+at;                                         \
            _ln = src->len-at;                                                           \
            __VA_ARGS__                                                                  \
            PARAS_NOTIFY(                                                                \
                "not a known valid instruction byte sequence"                            \
                " at offset %zu/%zu starting with 0x%02X",                               \
                at, src->len, src->ptr[at]);                                             \
            break;                                                                       \
        }                                                                                \
        return at;                                                                       \
    })                                                                                   \
                                                                                         \
    sz paras_asmbl_##__name(buf cref src, buf ref res) _paras_declonly({                 \
        static bool const _disasm = false;                                               \
        static u8 const* const bytes;                                                    \
        unsigned _ln = 0;                                                                \
        sz at = 0, loc;                                                                  \
        while (at < src->len) {                                                          \
            _ln++;                                                                       \
            while (at < src->len && strchr(" \t\n", src->ptr[at])) {                     \
                if ('\n' == src->ptr[at]) _ln++;                                         \
                at++;                                                                    \
            }                                                                            \
            char const* const word = (char*)src->ptr+at;                                 \
            while (at < src->len && !strchr(" \t\n", src->ptr[at])) at++;                \
            while (at < src->len && strchr(" \t", src->ptr[at])) at++;                   \
            if (at < src->len && ';' != *word) {                                         \
                __VA_ARGS__                                                              \
                while (at < src->len && '\n' != src->ptr[at]) at++;                      \
                PARAS_NOTIFY(                                                            \
                    "unknown mnemonic (%.*s) or wrong operands line %u",                 \
                    (unsigned)strcspn(word, " \t\n;"), word, _ln);                       \
                return 0;                                                                \
            }                                                                            \
            while (at < src->len && '\n' != src->ptr[at]) at++; /* XXX: dead code? */    \
        }                                                                                \
        return at;                                                                       \
    })

#endif // __PARAS_H__
