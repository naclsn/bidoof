#include <stdlib.h>

char* line_read(void);
void line_free(void);
void line_compgen(void (*)(char* line, size_t point));

#ifdef LINE_IMPLEMENTATION
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static inline bool setraw(struct termios* pst) {
    if (tcgetattr(STDIN_FILENO, pst)) return false;
    struct termios raw = *pst;
    raw.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    raw.c_oflag &= ~OPOST;
    raw.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    raw.c_cflag &= ~(CSIZE | PARENB);
    raw.c_cflag |= CS8;
    return 0 == tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

static inline void rstraw(struct termios* pst) {
    tcsetattr(STDIN_FILENO, TCSANOW, pst);
}

static void (*_compgen)(char* line, size_t point) = NULL;

static char* _hist_ls[128] = {0};
static size_t _hist_at = 0;
#define _hist_ln (128)

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

    char c;
    while (true) {
        switch (c = getchar()) {
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
                free(s);
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
                // TODO: call compgen
                (void)_compgen;
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
                // TODO: search
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
                    s[k] = p;
                    for (; k != i; k--) putchar('\b');
                }
        } // switch c
    } // while true

done:
    rstraw(&t);
    return s;
}

void line_free(void) {
    // XXX: segfault
    for (size_t k = 0; k < _hist_ln; k++)
        _hist_ls[k] = (free(_hist_ls[k]), NULL);
}

void line_compgen(void (*comp)(char*, size_t)) {
    _compgen = comp;
}

#endif // LINE_IMPLEMENTATION
