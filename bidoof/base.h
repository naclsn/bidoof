#ifndef __BIDOOF_H__
#define __BIDOOF_H__

#ifdef BIDOOF_T_IMPLEMENTATION
#undef BIDOOF_IMPLEMENTATION
#endif

#include <dirent.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/dyarr.h"

#define countof(__a) (sizeof(__a)/sizeof*(__a))

#define mkbuf(__c) (buf){.ptr= (u8*)__c, .len= strlen(__c)}
#define mkbufa(...) (buf){.ptr= (u8[])__VA_ARGS__, .len= countof(((u8[])__VA_ARGS__))}
#define mkbufsl(__p, __st, __ed) (buf){.ptr= (u8*)(__p)+(__st), .len= (__ed)-(__st)}
#define mkbrmapa(...) (brmap){.ptr= (brmap_en[])__VA_ARGS__, .len= countof(((brmap_en[])__VA_ARGS__))}

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

typedef struct brmap_en {
    sz start, stop;
    char const* const name;
} brmap_en;
typedef dyarr(brmap_en) brmap;

void xxd(buf const b, sz const l);
void xxdiff(buf const a, buf const b, sz const l);
void xxdhi(buf const b, brmap const m);
char const* binstr(u64 n, unsigned const w);
void putb(char opref h, buf const b, char opref t);
void putbhi(buf const b, char cref ref keywords, char cref ref string_pairs, char cref ref comment_pairs);

#define HI_C_KEYWORDS      "auto", "bool", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "false", "float", "for", "goto", "if", "inline", "int", "long", "register", "restrict", "return", "short", "signed", "sizeof", "static", "struct", "switch", "true", "typedef", "typeof", "union", "unsigned", "void", "volatile", "while", "NULL", "size_t", "int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t", "int64_t", "uint64_t"
#define HI_C_STRING_PAIRS  "\"", "\"", "'", "'"
#define HI_C_COMMENT_PAIRS "/*", "*/", "//", "\n", "#", "\n"
#define HI_LUA_KEYWORDS      "and", "break", "do", "else", "elseif", "end", "false", "for", "function", "if", "in", "local", "nil", "not", "or", "repeat", "return", "then", "true", "until", "while"
#define HI_LUA_STRING_PAIRS  "\"", "\"", "'", "'", "[[", "]]", "[=[", "]=]", "[==[", "]==]", "[===[", "]===]"
#define HI_LUA_COMMENT_PAIRS "--", "\n", "--[[", "]]", "--[=[", "]=]", "--[==[", "]==]", "--[===[", "]===]"
#define HI_PY_KEYWORDS      "False", "await", "else", "import", "pass", "None", "break", "except", "in", "raise", "True", "class", "finally", "is", "return", "and", "continue", "for", "lambda", "try", "as", "def", "from", "nonlocal", "while", "assert", "del", "global", "not", "with", "async", "elif", "if", "or", "yield"
#define HI_PY_STRING_PAIRS  "\"", "\"", "'", "'", "\"\"\"", "\"\"\"", "'''", "'''", "f\"", "\"", "f'", "'", "f\"\"\"", "\"\"\"", "f'''", "'''"
#define HI_PY_COMMENT_PAIRS "#", "\n"
#define HI_JS_KEYWORDS      "break", "case", "catch", "class", "const", "continue", "debugger", "default", "delete", "do", "else", "export", "extends", "false", "finally", "for", "function", "if", "import", "in", "instanceof", "new", "null", "return", "super", "switch", "this", "throw", "true", "try", "typeof", "var", "void", "while", "with", "let", "static", "yield", "await"
#define HI_JS_STRING_PAIRS  "\"", "\"", "'", "'", "`", "`"
#define HI_JS_COMMENT_PAIRS "/*", "*/", "//", "\n", "#!", "\n"

void buf_free(buf const self);

buf bufcpy(buf const other);
void bufcat(buf ref to, buf const other);
buf bufbeg(buf const b, buf const w);
buf bufend(buf const b, buf const w);
buf buftrimbeg(buf const b, char cref w);
buf buftrimend(buf const b, char cref w);

long long parsenum(buf const b, sz opref ends);

u16 peek16le(buf const b, sz const k);
u32 peek32le(buf const b, sz const k);
u64 peek64le(buf const b, sz const k);
void poke16le(buf ref b, sz const k, u16 const v);
void poke32le(buf ref b, sz const k, u32 const v);
void poke64le(buf ref b, sz const k, u64 const v);
u16 peek16be(buf const b, sz const k);
u32 peek32be(buf const b, sz const k);
u64 peek64be(buf const b, sz const k);
void poke16be(buf ref b, sz const k, u16 const v);
void poke32be(buf ref b, sz const k, u32 const v);
void poke64be(buf ref b, sz const k, u64 const v);

buf file_read(buf const path);
void file_write(buf const path, buf const b);
buf dir_list(buf const path);
buf path_basename(buf const path);
buf path_dirname(buf const path);
void path_join(buf ref to, buf const other);

#define swapn(__x, __y, __sz) do {  \
    sz _sz = __sz;                  \
    u8 tmp[__sz];                   \
    memcpy(tmp, &(__y), _sz);       \
    memcpy(&(__y), &(__x),  _sz);   \
    memcpy(&(__x), tmp, _sz);       \
} while (0)
#define swap(__x, __y) swapn((__x), (__y), sizeof(__x))

#define with_buf_as_write_file(__buf, __name) for (            \
    FILE* _hid_##__name = tmpfile(), * __name = _hid_##__name  \
        ? _hid_##__name                                        \
        : (exitf("could not open a temporary file"), NULL);    \
    _hid_##__name;                                             \
    ((__buf).ptr = malloc((__buf).len=ftell(_hid_##__name)))   \
        ? fseek(_hid_##__name, 0, SEEK_SET),                   \
          fread((__buf).ptr, 1, (__buf).len, _hid_##__name)    \
        : (exitf("OOM"), 0),                                   \
    fclose(_hid_##__name), _hid_##__name = NULL)

#define with_buf_as_read_file(__buf, __name, ...) for (        \
    FILE* _hid_##__name = tmpfile(), * __name = _hid_##__name  \
        ? fwrite((__buf).ptr, 1, (__buf).len, _hid_##__name),  \
          fseek(_hid_##__name, 0, SEEK_SET),                   \
          _hid_##__name                                        \
        : (exitf("could not open a temporary file"), NULL);    \
    _hid_##__name;                                             \
    fclose(_hid_##__name), _hid_##__name = NULL)

#define for_lines(__sl, __inbuf) for (                                            \
    buf _inbuf = (__inbuf), __sl = {.ptr= memchr(_inbuf.ptr, '\n', _inbuf.len)};  \
    __sl.ptr && (__sl.len = __sl.ptr - _inbuf.ptr, __sl.ptr = _inbuf.ptr);        \
    __sl.ptr = memchr(_inbuf.ptr+= __sl.len+1, '\n', _inbuf.len-= __sl.len+1))

#define _try_catch_scope_x(__n) _try_catch_scope_##__n
#define _try_catch_scope(__n) _try_catch_scope_x(__n)
#define try_catch(...)                                                 \
    if (!dyarr_push(&catch_stack)) notif("OOM"), exit(1);              \
    bool _try_catch_scope(__LINE__) = true;                            \
    if (!setjmp(catch_stack.ptr[catch_stack.len-1])) for (             \
        __VA_ARGS__;                                                   \
        _try_catch_scope(__LINE__) ? catch_stack.len--, true : false;  \
        _try_catch_scope(__LINE__) = false)

#define _HERE_STR(__ln) #__ln
#define _HERE_XSTR(__ln) _HERE_STR(__ln)
#define HERE(__fmt, ...) __FILE__ ":" _HERE_XSTR(__LINE__) ": (in %s) " __fmt "%c", __func__, __VA_ARGS__ '\n'
#define exitf(...) (notif(__VA_ARGS__), catch_stack.len ? longjmp(catch_stack.ptr[--catch_stack.len], 1) : exit(EXIT_FAILURE))
#define notif(...) (notify_stream ? fprintf(notify_stream, HERE(__VA_ARGS__,)), fputc('\n', notify_stream) : 0)


#ifdef BIDOOF_IMPLEMENTATION
#define _extern(__ty, __nm, ...) __ty __nm = __VA_ARGS__
#else
#define _extern(__ty, __nm, ...) extern __ty __nm
#endif
_extern(dyarr(jmp_buf), catch_stack, {0});
_extern(FILE*, notify_stream, NULL);
#undef _extern

#ifdef BIDOOF_IMPLEMENTATION

void xxd(buf const b, sz const ln) {
    if (0 == b.len) return;
    for (sz j = 0; j < ln && j < (b.len-1)/16+1; j++) {
        printf("%07zx0:   ", j);
        for (sz i = 0; i < 16; i++) {
            sz const k = i+16*j;
            if (b.len <= k) printf("   ");
            else printf("%02X ", b.ptr[k]);
        }
        printf("       ");
        for (sz i = 0; i < 16; i++) {
            sz const k = i+16*j;
            if (b.len <= k) break;
            char const it = b.ptr[k];
            printf("%c", ' ' <= it && it <= '~' ? it : '.');
        }
        printf("\n");
    }
}

void xxdiff(buf const l, buf const r, sz const ln) {
    sz const len = l.len < r.len ? r.len : l.len;
    if (0 == len) return;
    sz first = -1;
    for (sz j = 0; j < ln && j < (len-1)/16+1; j++) {
        printf("%07zx0:   ", j);
        for (sz i = 0; i < 16; i++) {
            sz const k = i+16*j;
            if (l.len <= k) printf("   ");
            else {
                bool const diff = r.len <= k || l.ptr[k] != r.ptr[k];
                if (diff) printf("\x1b[31m");
                printf("%02X ", l.ptr[k]);
                if (diff) printf("\x1b[m");
                if (diff && (sz)-1 == first) first = k;
            }
        }
        printf("       ");
        for (sz i = 0; i < 16; i++) {
            sz const k = i+16*j;
            if (r.len <= k) printf("   ");
            else {
                bool const diff = l.len <= k || l.ptr[k] != r.ptr[k];
                if (diff) printf("\x1b[32m");
                printf("%02X ", r.ptr[k]);
                if (diff) printf("\x1b[m");
                if (diff && (sz)-1 == first) first = k;
            }
        }
        printf("\n");
    }
    if ((sz)-1 != first) printf("first diff at offset %zu\n", first);
}

void xxdhi(buf const b, brmap const m) {
    static char const* const colors[] = {"\x1b[31m", "\x1b[32m", "\x1b[33m", "\x1b[34m", "\x1b[35m", "\x1b[36m"};
    unsigned cs = 0, cc = 2;
    bool isin = false;
    if (0 == b.len) return;
    for (sz j = 0; j < (b.len-1)/16+1; j++) {
        unsigned pcs = cs, pcc = cc;
        bool pisin = isin;
        printf("%07zx0:   %s", j, isin ? colors[cc] : "");
        for (sz i = 0; i < 16; i++) {
            sz const k = i+16*j;
            if (b.len <= k) printf("   ");
            else {
                if (cs < m.len && isin && !(isin = m.ptr[cs].stop != k)) {
                    printf("\x1b[m");
                    cs++;
                    if (countof(colors) == ++cc) cc = 0;
                }
                if (cs < m.len && !isin && (isin = m.ptr[cs].start == k)) printf("%s", colors[cc]);
                printf("%02X ", b.ptr[k]);
            }
        }
        cs = pcs, cc = pcc;
        isin = pisin;
        unsigned encount = 0;
        printf("\x1b[m       %s", isin ? colors[cc] : "");
        for (sz i = 0; i < 16; i++) {
            sz const k = i+16*j;
            if (b.len <= k) printf(" ");
            else {
                if (cs < m.len && isin && !(isin = m.ptr[cs].stop != k)) {
                    printf("\x1b[m");
                    cs++;
                    if (countof(colors) == ++cc) cc = 0;
                }
                if (cs < m.len && !isin && (isin = m.ptr[cs].start == k)) printf("%s", colors[cc]), encount++;
                printf("%c", ' ' <= b.ptr[k] && b.ptr[k] <= '~' ? b.ptr[k] : '.');
            }
        }
        printf("\x1b[m       ");
        while (encount--) {
            while (16*j == m.ptr[pcs].stop) {
                pcs++;
                if (countof(colors) == ++pcc) pcc = 0;
            }
            printf("%s%s (%zub)\x1b[m ", colors[pcc], m.ptr[pcs].name, m.ptr[pcs].stop-m.ptr[pcs].start);
            pcs++;
            if (countof(colors) == ++pcc) pcc = 0;
        }
        printf("\n");
    }
}

void putb(char opref h, buf const b, char opref t) {
    if (h) printf("%s", h);
    printf("%.*s", (unsigned)b.len, b.ptr);
    if (t) printf("%s", t);
}

void putbhi(buf const b, char cref ref keywords, char cref ref string_pairs, char cref ref comment_pairs) {
    bool canstartw = true;
    for (sz k = 0; k < b.len; k++) {
        if (canstartw) for (char cref* w = keywords; *w; w++) {
            sz const len = strlen(*w);
            if (( (k+len < b.len && !(
                            ('0' <= b.ptr[k+len] && b.ptr[k+len] <= '9') ||
                            ('A' <= b.ptr[k+len] && b.ptr[k+len] <= 'Z') ||
                            ('a' <= b.ptr[k+len] && b.ptr[k+len] <= 'z') ||
                            '_' == b.ptr[k+len]))
                        || k+len == b.len )
                    && !memcmp(*w, b.ptr+k, len)) {
                printf("\x1b[34m%s\x1b[m", *w);
                k+= len-1;
                canstartw = false;
                goto next;
            }
        }
        for (char cref* t = string_pairs; *t; t+= 2) if (!memcmp(*t, b.ptr+k, strlen(*t))) {
            printf("\x1b[36m%c", **t);
            sz const elen = strlen(*++t);
            if (1 == elen && **t == **(t-1)) while (++k < b.len && **t != b.ptr[k]) {
                putchar(b.ptr[k]);
                if ('\\' == b.ptr[k]) putchar(b.ptr[++k]);
            } else while (++k+elen <= b.len && memcmp(*t, b.ptr+k, elen)) putchar(b.ptr[k]);
            printf("%s\x1b[m", *t);
            k+= elen-1;
            canstartw = true;
            goto next;
        }
        for (char cref* t = comment_pairs; *t; t+= 2) if (!memcmp(*t, b.ptr+k, strlen(*t))) {
            printf("\x1b[32m%c", **t);
            sz const elen = strlen(*++t);
            while (++k+elen <= b.len && memcmp(*t, b.ptr+k, elen)) putchar(b.ptr[k]);
            printf("%s\x1b[m", *t);
            k+= elen-1;
            canstartw = true;
            goto next;
        }
        if (canstartw && (('0' <= b.ptr[k] && b.ptr[k] <= '9') || '.' == b.ptr[k])) {
            printf("\x1b[33m");
            while (k < b.len && (
                        ('0' <= b.ptr[k] && b.ptr[k] <= '9') ||
                        ('A' <= b.ptr[k] && b.ptr[k] <= 'F') ||
                        ('a' <= b.ptr[k] && b.ptr[k] <= 'f') ||
                        'b' == b.ptr[k] || 'o' == b.ptr[k] || 'x' == b.ptr[k] ||
                        '_' == b.ptr[k])) putchar(b.ptr[k++]);
            if ('.' == b.ptr[k]) while (k < b.len && (
                        ('0' <= b.ptr[k] && b.ptr[k] <= '9') ||
                        ('A' <= b.ptr[k] && b.ptr[k] <= 'F') ||
                        ('a' <= b.ptr[k] && b.ptr[k] <= 'f') ||
                        '_' == b.ptr[k])) putchar(b.ptr[k++]);
            k--;
            printf("\x1b[m");
            canstartw = false;
            goto next;
        }
        canstartw = !(
                ('0' <= b.ptr[k] && b.ptr[k] <= '9') ||
                ('A' <= b.ptr[k] && b.ptr[k] <= 'Z') ||
                ('a' <= b.ptr[k] && b.ptr[k] <= 'z') ||
                '_' == b.ptr[k]);
        putchar(b.ptr[k]);
    next:;
    }
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

void buf_free(buf const self) {
    if (self.cap) free(self.ptr);
}

buf bufcpy(buf const other) {
    buf r = {.ptr= malloc(other.len), .len= other.len, .cap= other.len};
    if (!r.ptr) exitf("OOM");
    memcpy(r.ptr, other.ptr, other.len);
    return r;
}

void bufcat(buf ref to, buf const other) {
    dyarr_replace(to, to->len, 0, &other);
}

buf bufbeg(buf const b, buf const w) {
    for (sz i = w.len; i < b.len; i++) if (!memcmp(b.ptr+i-w.len, w.ptr, w.len)) return mkbufsl(b.ptr, i-w.len, b.len);
    return (buf){0};
}

buf bufend(buf const b, buf const w) {
    for (sz i = w.len; i < b.len; i++) if (!memcmp(b.ptr+b.len-i, w.ptr, w.len)) return mkbufsl(b.ptr, 0, b.len-i);
    return (buf){0};
}

buf buftrimbeg(buf const b, char cref w) {
    buf r = b;
    for (; r.ptr < b.ptr+b.len; r.ptr++) if (!strchr(w, *r.ptr)) break;
    return r;
}

buf buftrimend(buf const b, char cref w) {
    buf r = b;
    for (; r.len; r.len--) if (!strchr(w, r.ptr[r.len-1])) break;
    return r;
}

long long parsenum(buf const b, sz opref ends) {
    long long r = 0;
    sz k = 0;
    if (b.len) {
        bool const minus = '-' == b.ptr[k];
        if (minus || '+' == b.ptr[k]) k++;
        unsigned shft = 0;
        char const* dgts = "0123456789";
        if (k < b.len && '0' == b.ptr[k]) switch (b.ptr[++k]|32) {
            case 'b': k++; shft = 1; dgts = "01";               break;
            case 'o': k++; shft = 3; dgts = "01234567";         break;
            case 'x': k++; shft = 4; dgts = "0123456789abcdef"; break;
        }
        char const* v = strchr(dgts, b.ptr[k]|32);
        if (!v) exitf("expected digit in \"%s\" but got '%c'", dgts, b.ptr[k-1]);
        do r = (!shft ? r*10 : r<<shft) + (v-dgts);
        while (++k < b.len && (v = strchr(dgts, b.ptr[k]|32)));
        if (minus) r*= -1;
    }
    if (ends) *ends = k;
    return r;
}

u16 peek16le(buf const b, sz const k) {
    return (u16)b.ptr[k]
         | (u16)b.ptr[k+1]<<8;
}
u32 peek32le(buf const b, sz const k) {
    return (u32)b.ptr[k]
         | (u32)b.ptr[k+1]<<8
         | (u32)b.ptr[k+2]<<16
         | (u32)b.ptr[k+3]<<24;
}
u64 peek64le(buf const b, sz const k) {
    return (u64)b.ptr[k]
         | (u64)b.ptr[k+1]<<8
         | (u64)b.ptr[k+2]<<16
         | (u64)b.ptr[k+3]<<24
         | (u64)b.ptr[k+4]<<32
         | (u64)b.ptr[k+5]<<40
         | (u64)b.ptr[k+6]<<48
         | (u64)b.ptr[k+7]<<56;
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

u16 peek16be(buf const b, sz const k) {
    return (u16)b.ptr[k]<<8
         | (u16)b.ptr[k+1];
}
u32 peek32be(buf const b, sz const k) {
    return (u32)b.ptr[k]<<24
         | (u32)b.ptr[k+1]<<16
         | (u32)b.ptr[k+2]<<8
         | (u32)b.ptr[k+3];
}
u64 peek64be(buf const b, sz const k) {
    return (u64)b.ptr[k]<<56
         | (u64)b.ptr[k+1]<<48
         | (u64)b.ptr[k+2]<<40
         | (u64)b.ptr[k+3]<<32
         | (u64)b.ptr[k+4]<<24
         | (u64)b.ptr[k+5]<<16
         | (u64)b.ptr[k+6]<<8
         | (u64)b.ptr[k+7];
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

buf file_read(buf const path) {
    buf r = {0};
    char local[path.len+1];
    memcpy(local, path.ptr, path.len);
    local[path.len] = '\0';
    FILE* f = fopen(local, "rb");
    if (!f) exitf("could not open file %.*s", (int)path.len, path.ptr);
    if (0 != fseek(f, 0, SEEK_END)) exitf("could not read file %.*s", (int)path.len, path.ptr);
    if (!(r.ptr = malloc(r.len = r.cap = ftell(f)))) exitf("OOM");
    fseek(f, 0, SEEK_SET);
    fread(r.ptr, 1, r.len, f);
    fclose(f);
    return r;
}

void file_write(buf const path, buf const b) {
    char local[path.len+1];
    memcpy(local, path.ptr, path.len);
    local[path.len] = '\0';
    FILE* f = fopen(local, "wb");
    if (!f) exitf("could not open file %.*s", (int)path.len, path.ptr);
    fwrite(b.ptr, 1, b.len, f);
    fclose(f);
}

buf dir_list(buf const path) {
    char local[path.len+1];
    memcpy(local, path.ptr, path.len);
    local[path.len] = '\0';
    buf r = {0};
    DIR* d = opendir(local);
    struct dirent const* en;
    while ((en = readdir(d))) if (strcmp(".", en->d_name) && strcmp("..", en->d_name)) {
        bufcat(&r, mkbufsl(en->d_name, 0, strlen(en->d_name)+1));
        r.ptr[r.len-1] = '\n';
    }
    closedir(d);
    return r;
}

buf path_basename(buf const path) {
    if (!path.len) return path;
    sz const l = path.len-1;
    bool const e = '/' == path.ptr[l] || '\\' == path.ptr[l];
    sz k;
    for (k = e; k < path.len; k++) if ('/' == path.ptr[l-k] || '\\' == path.ptr[l-k]) break;
    return mkbufsl(path.ptr, l-(k-1), path.len-e);
}

buf path_dirname(buf const path) {
    if (!path.len) return path;
    sz const l = path.len-1;
    bool const e = '/' == path.ptr[l] || '\\' == path.ptr[l];
    sz k;
    for (k = e; k < path.len; k++) if ('/' == path.ptr[l-k] || '\\' == path.ptr[l-k]) break;
    return mkbufsl(path.ptr, 0, path.len-1-k);
}

void path_join(buf ref to, buf const other) {
    bool const
        a = to->len && '/' == to->ptr[to->len-1],
        b = other.len && '/' == other.ptr[0];
    if (a && b) to->len--;
    else if (!a && !b) *dyarr_push(to) = '/';
    bufcat(to, other);
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

#define make_arg_buf(__name, __information)                                                 \
    buf const __name = (_is_h ? puts("\t" #__name ":\t" __information), (buf){0}            \
                    : !argc ? exitf("expected value for argument '" #__name "'"), (buf){0}  \
                    : (argc--, argv++, (buf){.ptr= (u8*)argv[-1], .len= strlen(argv[-1])})  \
                    );
#define make_arg_int(__name, __information)                                            \
    int const __name = (_is_h ? puts("\t" #__name ":\t" __information), 0              \
                     : !argc ? exitf("expected number for argument '" #__name "'"), 0  \
                     : (argc--, atoi(*argv++))                                         \
                     );

#define make_cmd(__invocation, __description, ...)                    \
    if (_is_h) puts("\t" #__invocation ":\t" __description);          \
    else {                                                            \
        static char const invocation[] = #__invocation;               \
        if (!strncmp(invocation, *argv, strcspn(invocation, " \t("))  \
                && !(*argv)[strcspn(invocation, " \t(")]) {           \
            argc--, argv++;                                           \
            if (argc && !strcmp("-h", *argv)) {                       \
                puts(#__invocation ":\t" __description);              \
                static bool const _is_h = true;                       \
                __VA_ARGS__                                           \
                return 0;                                             \
            }                                                         \
            static bool const _is_h = false;                          \
            __VA_ARGS__                                               \
            __invocation;                                             \
            return 0;                                                 \
        }                                                             \
    }

#define make_main(__summary, ...)                  \
    int main(int argc, char** argv) {              \
        notify_stream = stdout;                    \
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
    __tname __tname##_parse(buf const buf) {       \
        __tname r = {0};                           \
        sz at = 0;                                 \
        if (!bipa_parse_##__tname(&r, &buf, &at))  \
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
    __tname __tname##_parse(buf const buf);        \
    void __tname##_free(__tname cref self);
#endif
