#include "lang.h"

// <script> ::= <var> '=' <expr> {';' <var> '=' <expr>} [';']
// <expr> ::= <atom> | <fun> {<expr>} | <expr> ',' <expr>
// <atom> ::= <str> | <num> | <lst> | <fun> | <sym> | <var> | '(' <expr> ')'
//
// <comment> ::= '#' /.*/ '\n'
//
// <str> ::= '"' /[^"]/ '"'
// <num> ::= ['-'] /0x[0-9A-Fa-f_]+|0o[0-8_]+|0b[01_]+|[0-9_](\.[0-9_])?|'[^']'/
// <lst> ::= '{' [<atom> {',' <atom>}] '}'
// <fun> ::= /[A-Z][0-9A-Za-z]+/
// <sym> ::= ':' /[0-9A-Za-z_]+/
// <var> ::= /[a-z_][0-9a-z_]+/
//
// --- syntax extensions
//
// <atom> ::= ... | '(=' <math> ')' | '($' <bind> ')'
//
// <math> ::
//     = <unop> <atom>
//     | <atom> <binop> <atom>
//     | <atom> '?' <atom> ':' <atom>
//     | <atom> '[' <atom> [':' [<atom>]] ']'
// <unop> ::= '+' '-' '!' '~'
// <binop> ::= '+' '-' '*' '/' '%' '//' '**' '==' '!=' '>' '<' '>=' '<=' '<=>' '&' '|' '^' '<<' '>>'
//
// <bind> ::= <fun> {<atom>}

typedef struct Pars { char* s; sz i; Sym t; } Pars;

bool _lex(Pars* self) {
#define AT              (self->s+self->i)
#define IN(__lo, __hi)  (__lo <= c && c <= __hi)
    char c;
    while (' ' == (c = self->s[self->i])
            || '\t' == c
            || '\n' == c
            || '\r' == c
            ) self->i++;

    self->t.ptr = AT;
    self->t.len = 0;

    // (when parsing a number)
    unsigned base = 10;

    switch (c) {
        case '(': case ')':
        case '{': case '}':
        case ',': case ';':
        case '=':
            self->t.len++;
            self->i++;
            return true;

        case '"': {
            char* end = AT;
            do end = strchr(end+1, '"');
            while (end && '\\' == end[-1]);
            if (!end) return false;
            self->t.len = end+1 - AT;
            self->i+= self->t.len;
        } return true;

        case '-':
            self->i++;
            if (!*AT) return false;
            // fall through
        case '0':
            self->t.len++;
            self->i++;
            switch (*AT) {
                case 'b': base =  2; break;
                case 'o': base =  8; break;
                case 'x': base = 16; break;
                case '.': case '_':
                case '0'...'9': break;
                case '\0': return true;
                default: return false;
            }
            // fall through
        case '1'...'9': {
            do c = self->s[++self->t.len, ++self->i];
            while ('_' == c
                    || (  2 == base && (IN('0', '1')) )
                    || (  8 == base && (IN('0', '7')) )
                    || ( 10 == base && (IN('0', '9')) )
                    || ( 16 == base && (IN('0', '9') || IN('A', 'F') || IN('a', 'f')) )
                    );
            if (10 == base && '.' == c) {
                do c = self->s[++self->t.len, ++self->i];
                while ('_' == c || IN('0', '9'));
            }
            if (10 != base && 2 == self->t.len) return false;
            // better report as syntax error, ~~even tho~~
            // **because** it could be valid
            if (IN('2', '9') || IN('A', 'Z') || IN('a', 'z')) return false;
        } return true;

        case '\'': {
            char* end = AT;
            do end = strchr(end+1, '\'');
            while (end && '\\' == end[-1]);
            if (!end) return false;
            self->t.len = end+1 - AT;
            self->i+= self->t.len;
        } return true;

        case 'A'...'Z':
            do c = self->s[++self->t.len, ++self->i];
            while (IN('0', '9') || IN('A', 'Z') || IN('a', 'z'));
            return true;

        case ':':
            do c = self->s[++self->t.len, ++self->i];
            while ('_' == c || IN('0', '9') || IN('A', 'Z') || IN('a', 'z'));
            if (1 == self->t.len) return false;
            return true;

        case '_':
        case 'a'...'z':
            do c = self->s[++self->t.len, ++self->i];
            while ('_' == c || IN('0', '9') || IN('a', 'z'));
            return true;
    }

    return false;
#undef IN
#undef AT
}

void _print_location(Pars* self, char* reason) {
    sz lineNr = 1, colNr = 1;
    sz lineStart = 0, lineEnd = 0;

    for (sz k = 0; k < self->i; k++) {
        colNr++;
        if ('\n' == self->s[k]) {
            lineNr++;
            colNr = 1;
            lineStart = k+1;
        }
    }

    char* nl = strchr(self->s+self->i, '\n');
    if (nl) lineEnd = nl - self->s;

    printf("%s %s:%zu:%zu\n", reason, "<script>", lineNr, colNr);
    if (lineEnd <= lineStart)
        printf("%4zu | %s\n", lineNr, self->s+lineStart);
    else
        printf("%4zu | %.*s\n", lineNr, (int)(lineEnd-lineStart), self->s+lineStart);
    printf("%4zu | %*s\n", lineNr+1, (int)colNr, "^");
}

bool _escape(Pars* self, u32* res) {
    (void)self;
    *res = 0;
    return false;
}

#define tok_is_str(__t) ('"' == (__t)->ptr[0])
#define tok_is_num(__t) ('\'' == (__t)->ptr[0]  \
            || ('0' <= (__t)->ptr[0] && (__t)->ptr[0] <= '9')  \
            || ('-' == (__t)->ptr[0] && '0' <= (__t)->ptr[1] && (__t)->ptr[1] <= '9'))
#define tok_is_fun(__t) ('A' <= (__t)->ptr[0] && (__t)->ptr[0] <= 'Z')
#define tok_is_var(__t) ('_' == (__t)->ptr[0] || ('a' <= (__t)->ptr[0] && (__t)->ptr[0] <= 'z'))
#define tok_is_sym(__t) (':' == (__t)->ptr[0])

bool lang_process(char* script, Scope* scope) {
    (void)scope;

    Pars p = {.s= script, .i= 0 };
    while (_lex(&p)) {
        printf("token <<%.*s>> (a %s)\n",
                (int)p.t.len, p.t.ptr,
                tok_is_str(&p.t) ? "str" :
                tok_is_num(&p.t) ? "num" :
                tok_is_fun(&p.t) ? "fun" :
                tok_is_var(&p.t) ? "var" :
                tok_is_sym(&p.t) ? "sym" :
                "punct");
        _print_location(&p, "TOKEN");
        puts("");
    }

    if (!p.t.ptr[0]) printf("end of script\n");
    else {
        printf("unexpected 0x%02X\n", p.s[p.i]);
        _print_location(&p, "ERROR");
    }

    return false;
}
