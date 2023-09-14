/// A small `readline`-like with only '\b', in the most unhinged free-form C.
/// ```c
/// char* line;
/// while (printf(">> "), line = line_read()) {
///     printf("echo: '%s'\n", line);
/// }
/// line_free();
/// ```
///
/// To access the history use `line_histget` and `line_histset`.
/// For completion see `line_compgen`.
///
/// The size of the history is fixed at compile-time; the default value is 128
/// entries, this can by changed by defining `_hist_ln`.
///
/// Make sure to only use LINE_IMPLEMENTATION once.
///
/// Maybe-interesting-note: as it only use `getchar` and `putchar` these can be
/// (re-)`#defined` to another target; in that case you will also need to have
/// ```c
/// #define tcgetattr(...) 0
/// #define tcsetattr(...) 0
/// ```

#if !defined(getchar) || !defined(putchar)
#include <stdlib.h>
#endif

char* line_read(void);
// this is a direct access to the history (careful with editing it)
char** line_histget(size_t* count);
// `lines` is NULL terminated; these are copied using the default allocator
void line_histset(char const* const* lines, size_t count);
// always call this after at least one `line_read`
void line_free(void);
// `words` should return a NULL-terminated list of completions insertable as-is
// `clean` can be NULL, otherwise it is called with the result afterward
void line_compgen(char const* const* (*words)(char* line, size_t point), void (*clean)(char const* const* words));

#ifdef LINE_IMPLEMENTATION
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static char const* const* (*_compgen_words)(char* line, size_t point) = NULL;
static void (*_compgen_clean)(char const* const* words) = NULL;

#ifndef _hist_ln
#define _hist_ln (128)
#endif
static char* _hist_ls[_hist_ln] = {0};
static size_t _hist_at = 0;

#undef CTRL
#define CTRL(x) (x&31)
#define ESC CTRL('[')
#define DEL 0x7f

#define IS_LETTER(__c) (('A' <= __c && __c <= 'Z') || ('a' <= __c && __c <= 'z'))
#define IS_WORD(__c) (('0' <= __c && __c <= '9') || IS_LETTER(__c))

#define WORD_BKW(__does) do {        \
        while (0 < i) {              \
            if (!IS_WORD(s[i-1])) {  \
                i--;                 \
                __does;              \
            } else break;            \
        }                            \
        while (0 < i) {              \
            if (IS_WORD(s[i-1])) {   \
                i--;                 \
                __does;              \
            } else break;            \
        }                            \
    } while (false)

#define WORD_FWD(__does) do {      \
        while (s[i]) {             \
            if (!IS_WORD(s[i])) {  \
                __does;            \
                i++;               \
            } else break;          \
        }                          \
        while (s[i]) {             \
            if (IS_WORD(s[i])) {   \
                __does;            \
                i++;               \
            } else break;          \
        }                          \
    } while (false)

char* line_read(void) {
    char* s;
    size_t i = 0;

    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term)) {
        static size_t const n = 64-12;
        s = calloc(n, 1);
        if (!s) return NULL;
        s[n-1] = ESC;

        char c = getchar();
        if (EOF == c) return NULL;

        while (true) {
            if (EOF == c || '\n' == c) {
                s['\r' == s[i-1] ? i-1 : i] = '\0';
                break;
            }

            if (ESC == s[i]) {
                size_t n = (i+1)*2+12;
                char* w = realloc(s, n);
                if (!w) break;
                s = w;
                memset(s+i+1, '\0', n-i-2);
                s[n-1] = ESC;
            }

            s[i++] = c;
            c = getchar();
        } // while true - breaks on eof/eol and alloc fails

        return free(_hist_ls[0]), _hist_ls[0] = s;
    } // if not tty

    {
        struct termios raw = term;
        raw.c_iflag&=~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
        raw.c_oflag&=~(OPOST);
        raw.c_lflag&=~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
        raw.c_cflag&=~(CSIZE | PARENB);
        raw.c_cflag|= (CS8);
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    }

    if (_hist_at) for (s = _hist_ls[_hist_at]; s[i]; i++) putchar(s[i]);
    else if (!_hist_ls[0] || _hist_ls[0][0]) {
        memmove(_hist_ls+1, _hist_ls, (_hist_ln-1)*sizeof(char*));
        static size_t const n = 64-12;
        _hist_ls[0] = s = calloc(n, 1);
        if (!s) goto done;
        s[n-1] = ESC;
    } else s = _hist_ls[0];

    bool reprocess = false;
    char c;
    while (true) {
        if (!reprocess) c = getchar();
        else reprocess = false;
        switch (c) {
            case ESC:
                switch (c = getchar()) {
                    case 'b':
                        WORD_BKW(putchar('\b'));
                        break;

                    case 'c':
                        if (s[i]) {
                            bool done = false;
                            WORD_FWD(putchar(IS_LETTER(s[i])
                                ? s[i] = (!done ? done = true, s[i]&~32 : s[i]|32)
                                : s[i]));
                        }
                        break;

                    case 'd':
                        if (s[i]) {
                            size_t j = i, k;
                            WORD_FWD();
                            for (k = j; s[i]; k++, i++) putchar(s[k] = s[i]);
                            s[k] = '\0';
                            for (; k != i; k++) putchar(' ');
                            for (; i != j; i--) putchar('\b');
                        }
                        break;

                    case 'f':
                        WORD_FWD(putchar(s[i]));
                        break;

                    case CTRL('H'):
                    case DEL:
                        if (i) {
                            size_t j = i, k;
                            WORD_BKW(putchar('\b'));
                            for (k = i; s[j]; k++, j++) putchar(s[k] = s[j]);
                            s[k] = '\0';
                            for (; k != j; k++) putchar(' ');
                            for (; k != i; k--) putchar('\b');
                        }
                        break;

                    case 'l':
                        WORD_FWD(putchar(IS_LETTER(s[i]) ? s[i]|= 32 : s[i]));
                        break;

                    case 't':
                        if (i) {
                            size_t j = i, bst, ben, fst, fen;
                            bool found = false;
                            WORD_BKW(if (!found && IS_LETTER(s[i])) { found = true; ben = i+1; });
                            bst = i;
                            i = j;
                            if (!found) break;
                            found = false;
                            WORD_FWD(if (!found && IS_LETTER(s[i])) { found = true; fst = i; });
                            fen = i;
                            if (!found) {
                                fst = bst;
                                fen = ben;
                                i = bst;
                                found = false;
                                WORD_BKW(if (!found && IS_LETTER(s[i])) { found = true; ben = i+1; });
                                bst = i;
                                i = j;
                                if (!found) break;
                            }
                            i = j;
                            size_t len = strlen(s);
                            char* cpy = malloc(len);
                            if (!cpy) break;
                            memcpy(cpy, s, len);
                            for (; i != bst; i--) putchar('\b');
                            for (size_t k = fst; k != fen; k++, i++) putchar(s[i] = cpy[k]);
                            for (size_t k = ben; k != fst; k++, i++) putchar(s[i] = cpy[k]);
                            for (size_t k = bst; k != ben; k++, i++) putchar(s[i] = cpy[k]);
                            free(cpy);
                        }
                        break;

                    case 'u':
                        WORD_FWD(putchar(IS_LETTER(s[i]) ? s[i]&=~32 : s[i]));
                        break;
                }
                break; // case ESC

            case CTRL(']'):
                for (c = getchar(); s[i] && c != s[i]; i++) putchar(s[i]);
                break;

            case CTRL('A'):
                for (; i; i--) putchar('\b');
                break;

            case CTRL('B'):
                if (i) i--, putchar('\b');
                break;

            case CTRL('C'):
                putchar('^');
                putchar('C');
                s[0] = '\0';
                // fall through
            case CTRL('M'):
            case CTRL('J'):
                if (s[0] && 1 < _hist_at) strcpy(_hist_ls[0], s);
                _hist_at = 0;
                // fall through
            case CTRL('O'):
                if (_hist_at) _hist_at--;
                putchar('\r');
                putchar('\n');
                goto done;

            case CTRL('D'):
                if ('\0' != *s) {
                    if (s[i]) {
                        size_t j = i++, k;
                        for (k = j; s[i]; k++, i++) putchar(s[k] = s[i]);
                        s[k] = '\0';
                        putchar(' ');
                        for (; i != j; i--) putchar('\b');
                    }
                    break;
                }
                // fall through
            case EOF:
                s = NULL;
                goto done;

            case CTRL('E'):
                for (; s[i]; i++) putchar(s[i]);
                break;

            case CTRL('F'):
                if (s[i]) putchar(s[i++]);
                break;

            case CTRL('H'):
            case DEL:
                if (i) {
                    size_t j = i-1, k;
                    putchar('\b');
                    for (k = j; s[i]; k++, i++) putchar(s[k] = s[i]);
                    s[k] = '\0';
                    putchar(' ');
                    for (; i != j; i--) putchar('\b');
                }
                break;

            case CTRL('I'):
                if (_compgen_words) {
                    char const* const* words = _compgen_words(s, i);
                    while (words && words[0]) {
                        size_t k, j = 0;
                        for (k = 0; words[k]; k++) {
                            size_t l = strlen(words[k]);
                            if (j < l) j = l;
                        }
                        for (k = i; ESC != s[k]; k++);
                        size_t n = (k+1)+12;
                        while (n < k+j) n*= 2;
                        n-= 12;
                        char* cpy = malloc(n);
                        if (!cpy) break;
                        memcpy(cpy, s, k);
                        if (n-k-1) memset(cpy+k+1, '\0', n-k-2);
                        cpy[n-1] = ESC;
                        s = cpy;
                        cpy = _hist_ls[_hist_at];
                        _hist_ls[_hist_at] = s;
                        reprocess = true;
                        k = 0;
                        j = i;
                        while (true) {
                            for (; j != i; i--) putchar('\b');
                            for (; words[k][i-j]; i++) putchar(s[i] = words[k][i-j]);
                            size_t l = 0;
                            bool z = false;
                            for (; cpy[j+l]; l++) {
                                z = z || !s[i+l];
                                putchar(s[i+l] = cpy[j+l]);
                            }
                            s[i+l] = '\0';
                            if (z) for (; s[i+l]; l++) putchar(' ');
                            for (; l; l--) putchar('\b');
                            if (CTRL('I') != (c = getchar())) break;
                            if (!words[++k]) k = 0;
                        }
                        free(cpy);
                        break;
                    } // if words (while, so that it can break)
                    if (_compgen_clean) _compgen_clean(words);
                }
                break;

            case CTRL('K'):
                if (s[i]) {
                    size_t j = i;
                    for (; s[j]; j++) putchar(' ');
                    for (; j != i; j--) putchar('\b');
                    s[i] = '\0';
                }
                break;

            case CTRL('N'):
                if (_hist_at) {
                    size_t p_l = strlen(s), n_l, k;
                    for (k = i; k; k--) putchar('\b');
                    s = _hist_ls[--_hist_at];
                    for (k = 0; s[k]; k++) putchar(s[k]);
                    n_l = k;
                    if (n_l < p_l) {
                        for (; k < p_l; k++) putchar(' ');
                        for (; n_l < k; k--) putchar('\b');
                    }
                    i = n_l;
                }
                break;

            case CTRL('P'):
                if (_hist_at+1 < _hist_ln && _hist_ls[_hist_at+1]) {
                    size_t p_l = strlen(s), n_l, k;
                    for (k = i; k; k--) putchar('\b');
                    s = _hist_ls[++_hist_at];
                    for (k = 0; s[k]; k++) putchar(s[k]);
                    n_l = k;
                    if (n_l < p_l) {
                        for (; k < p_l; k++) putchar(' ');
                        for (; n_l < k; k--) putchar('\b');
                    }
                    i = n_l;
                }
                break;

            case CTRL('R'):
            case CTRL('S'):
                {
                    char sr_s[32];
                    size_t sr_i = 0;
                    bool rev = c == CTRL('R');
                    sr_s[0] = '\0';
                    sr_i = i;
                    for (; s[sr_i]; sr_i++) putchar(' ');
                    for (; sr_i; sr_i--) putchar('\b');
                    putchar('^');
                    putchar(rev ? 'R' : 'S');
                    putchar(':');
                    if (3 < i) for (; sr_i < i-3; sr_i++) putchar(' ');
                    for (; sr_i; sr_i--) putchar('\b');
                    sr_s[sr_i = 0] = '\0';
                    while (true) {
                        putchar('|');
                        size_t k = 0;
                        for (; s[k]; k++) putchar(s[k]);
                        k++, putchar(' ');
                        for (; k != i; k--) putchar('\b');
                        c = getchar();
                        if (' ' <= c && c <= '~' && sr_i <31) {
                            for (size_t k = i+1; k != 0; k--) putchar('\b');
                            putchar(sr_s[sr_i++] = c);
                        } else if ((DEL == c || CTRL('H') == c) && sr_i) {
                            for (size_t k = i+2; k != 0; k--) putchar('\b');
                            --sr_i;
                        } else {
                            reprocess = /*ESC != c &&*/ CTRL('G') != c && (rev ? CTRL('R') : CTRL('S')) != c;
                            break;
                        }
                        sr_s[sr_i] = '\0';
                        while (rev ? _hist_at+1 < _hist_ln && _hist_ls[_hist_at] : _hist_at) {
                            char* at = strstr(_hist_ls[_hist_at], sr_s);
                            if (!at) rev ? _hist_at++ : _hist_at--;
                            else {
                                size_t k = 1;
                                putchar(' ');
                                for (; s[k]; k++) putchar(' ');
                                for (; k; k--) putchar('\b');
                                i = at - (s = _hist_ls[_hist_at]);
                                break;
                            }
                        } // while strstr
                    } // while true - getchar (also breaks if not handled)
                    size_t k = i+sr_i+4;
                    for (; k; k--) putchar('\b');
                    for (; s[k]; k++) putchar(s[k]);
                    for (size_t j = 0; j != sr_i+4; j++, k++) putchar(' ');
                    for (; k != i; k--) putchar('\b');
                }
                break; // case ^R/^S

            case CTRL('T'):
                if (i) {
                    if (!s[i]) i--, putchar('\b');
                    putchar('\b');
                    char p = s[i-1];
                    putchar(s[i-1] = s[i]);
                    putchar(s[i++] = p);
                }
                break;

            case CTRL('U'):
                if (i) {
                    size_t j = i, k;
                    for (; i; i--) putchar('\b');
                    for (k = i; s[j]; k++, j++) putchar(s[k] = s[j]);
                    s[k] = '\0';
                    for (; k != j; k++) putchar(' ');
                    for (; j != i; j--) putchar('\b');
                }
                break;

            case CTRL('W'):
                if (i) {
                    size_t j = i, k;
                    while (0 < i && ' ' != s[i-1]) i--, putchar('\b');
                    for (k = i; s[j]; k++, j++) putchar(s[k] = s[j]);
                    s[k] = '\0';
                    for (; k != j; k++) putchar(' ');
                    for (; k != i; k--) putchar('\b');
                }
                break;

            default:
                if (' ' <= c && c <= '~') {
                    char p = s[i];
                    putchar(s[i++] = c);
                    size_t k = i;
                    while (p) {
                        char w = s[k];
                        putchar(s[k++] = p);
                        p = w;
                    }
                    if (ESC == s[k]) {
                        size_t n = (k+1)*2+12;
                        if (!(s = realloc(s, n))) goto done;
                        _hist_ls[_hist_at] = s;
                        memset(s+k+1, '\0', n-k-2);
                        s[n-1] = ESC;
                    }
                    s[k] = p;
                    for (; k != i; k--) putchar('\b');
                }
        } // switch c
    } // while true

done:
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    return s;
}

char** line_histget(size_t* count) {
    for (*count = 0; *count < _hist_ln && _hist_ls[*count]; (*count)++);
    return _hist_ls[0] && !_hist_ls[0][0] ? (*count)--, _hist_ls+1 : _hist_ls;
}

void line_histset(char const* const* lines, size_t count) {
    line_free();
    if (lines && count) for (size_t k = 0; count && k < _hist_ln; count--, k++) {
        size_t l = strlen(lines[k]), n = 64;
        while (n < l) n*= 2;
        n-= 12;
        if (!(_hist_ls[count-1] = calloc(n, 1))) break;
        memcpy(_hist_ls[count-1], lines[k], l+1);
        _hist_ls[count-1][n-1] = ESC;
    }
}

void line_free(void) {
    for (size_t k = _hist_at = 0; k < _hist_ln; k++) _hist_ls[k] = (free(_hist_ls[k]), NULL);
}

void line_compgen(char const* const* (*words)(char* line, size_t point), void (*clean)(char const* const* words)) {
    _compgen_words = words;
    _compgen_clean = clean;
}

#undef ESC
#undef DEL
#undef IS_LETTER
#undef IS_WORD
#undef WORD_BKW
#undef WORD_FWD
#endif // LINE_IMPLEMENTATION
