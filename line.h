#include <stdlib.h>

char* line_read(void);
char** line_hist(size_t* count);
void line_free(void);

// `words` should return a NULL-terminated list of completions insertable as-is
// `clean` can be NULL, otherwise it is called with the result afterward
void line_compgen(char const* const* (*words)(char const* line, size_t point), void (*clean)(char const* const* words));

#ifdef LINE_IMPLEMENTATION
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static inline bool setraw(struct termios* pst) {
    if (tcgetattr(STDIN_FILENO, pst)) return false;
    struct termios raw = *pst;
    raw.c_iflag&=~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    raw.c_oflag&=~(OPOST);
    raw.c_lflag&=~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    raw.c_cflag&=~(CSIZE | PARENB);
    raw.c_cflag|= (CS8);
    return 0 == tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

static inline void rstraw(struct termios* pst) {
    tcsetattr(STDIN_FILENO, TCSANOW, pst);
}

static char const* const* (*_compgen_words)(char const* line, size_t point) = NULL;
static void (*_compgen_clean)(char const* const* words) = NULL;

static char* _hist_ls[128] = {0};
static size_t _hist_at = 0;
#define _hist_ln (128)

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
    struct termios t;
    if (!setraw(&t)) {
        puts("NIY: pipe input not handled");
        return NULL;
    }

    char* s;
    size_t i = 0;

    if (_hist_at) for (s = _hist_ls[_hist_at]; s[i]; i++) putchar(s[i]);
    else if (!_hist_ls[0] || _hist_ls[0][0]) {
        memmove(_hist_ls+1, _hist_ls, (_hist_ln-1)*sizeof(char*));
        _hist_ls[0] = s = calloc(64, 1);
        s[63] = ESC;
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
                    if (words && words[0]) {
                        size_t k = 0, j = 0;
                        reprocess = true;
                        while (true) {
                            // TODO: remove previous (from i to i+j), insert words[k] at i
                            for (j = 0; words[k][j]; j++) putchar(words[k][j]);
                            if (CTRL('I') != (c = getchar())) break;
                            if (!words[++k]) k = 0;
                        }
                    }
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
                    // TODO: realloc and such
                    char p = s[i];
                    putchar(s[i++] = c);
                    size_t k = i;
                    while (p) {
                        char w = s[k];
                        putchar(s[k++] = p);
                        p = w;
                    }
                    s[k] = p;
                    for (; k != i; k--) putchar('\b');
                }
        } // switch c
    } // while true

done:
    rstraw(&t);
    return s;
}

char** line_hist(size_t* count) {
    for (*count = 0; *count < _hist_ln && _hist_ls[*count]; (*count)++);
    return !_hist_ls[0][0] ? (*count)--, _hist_ls+1 : _hist_ls;
}

void line_free(void) {
    for (size_t k = 0; k < _hist_ln; k++) _hist_ls[k] = (free(_hist_ls[k]), NULL);
}

void line_compgen(char const* const* (*words)(char const* line, size_t point), void (*clean)(char const* const* words)) {
    _compgen_words = words;
    _compgen_clean = clean;
}

#endif // LINE_IMPLEMENTATION
