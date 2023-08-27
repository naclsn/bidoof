#include "lang.h"
#include "exts.h"

// <script> ::= <var> '=' <expr> {';' <var> '=' <expr>} [';']
// <expr> ::
//      = <str> | <num> | <lst> | <fun> | <sym> | <var>
//      | <fun> <expr> {<expr>}
//      | '(' <expr> ')'
//      | <expr> ',' <expr>
//
// <comment> ::= '#' /.*/ '\n'
//
// <str> ::= /"[^"]"/
// <num> ::= /'[^']'/ | ['-'] /0x[0-9A-Fa-f_]+|0o[0-8_]+|0b[01_]+|[0-9_](\.[0-9_])?/
// <lst> ::= '{' [<expr> {',' <expr>}] '}'
// <fun> ::= /[A-Z][0-9A-Za-z]+/
// <sym> ::= ':' /[0-9A-Za-z_]+/
// <var> ::= /[a-z_][0-9a-z_]+/
//
// --- escape sequences
//
// '\a': 0x07
// '\b': 0x08
// '\e': 0x1B
// '\f': 0x0C
// '\n': 0x0A
// '\r': 0x0D
// '\t': 0x09
// '\v': 0x0B
// '\\': 0x5C
// '\'': 0x27
// '\"': 0x22
// '\xHH': 2 hex digits byte
// '\uHHHH': 4 hex digits codepoint below 0x10000
// '\UHHHHHHHH': 8 hex digits codepoint
// '\OOO': 3 oct digits (idk y tho, is there this much use for it?)
//
// --- syntax extensions
//
// <expr> ::= ... | '(=' <math> ')' | '($' <bind> ')'
//
// <math> ::
//     = <unop> <expr>
//     | <expr> <binop> <expr>
//     | <expr> '?' <expr> ':' <expr>
//     | <expr> '[' <expr> [':' [<expr>]] ']'
// <unop> ::= '+' '-' '!' '~'
// <binop> ::= '+' '-' '*' '/' '%' '//' '**' '==' '!=' '>' '<' '>=' '<=' '<=>' '&' '|' '^' '<<' '>>'
//
// <bind> ::= <fun> {<expr>}

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
    if (lineEnd <= lineStart) printf("%4zu | %s\n", lineNr, self->s+lineStart);
    else printf("%4zu | %.*s\n", lineNr, (int)(lineEnd-lineStart), self->s+lineStart);
    printf("%4zu | %*s\n", lineNr+1, (int)colNr, "^");
}

bool _escape(Sym* slice, u32* res) {
    *res = 0;

    switch (*slice->ptr++) {
        case 'a': *res = 0x07; return true;
        case 'b': *res = 0x08; return true;
        case 'e': *res = 0x1B; return true;
        case 'f': *res = 0x0C; return true;
        case 'n': *res = 0x0A; return true;
        case 'r': *res = 0x0D; return true;
        case 't': *res = 0x09; return true;
        case 'v': *res = 0x0B; return true;
        case '\\': *res = 0x5C; return true;
        case '\'': *res = 0x27; return true;
        case '"': *res = 0x22; return true;

        case 'x': // 2 hex digits byte
            break;

        case 'u': // 4 hex digits codepoint below 0x10000
            break;

        case 'U': // 8 hex digits codepoint
            break;

        case '0'...'7': // 3 oct digits (idk y tho, is there this much use for it?)
            break;
    }

    return false;
}

#define tok_is_str(__t) ('"' == (__t).ptr[0])
#define tok_is_num(__t) ('\'' == (__t).ptr[0]  \
            || ('0' <= (__t).ptr[0] && (__t).ptr[0] <= '9')  \
            || ('-' == (__t).ptr[0] && '0' <= (__t).ptr[1] && (__t).ptr[1] <= '9'))
#define tok_is_fun(__t) ('A' <= (__t).ptr[0] && (__t).ptr[0] <= 'Z')
#define tok_is_sym(__t) (':' == (__t).ptr[0])
#define tok_is_var(__t) ('_' == (__t).ptr[0] || ('a' <= (__t).ptr[0] && (__t).ptr[0] <= 'z'))
#define tok_is(__cstr, __t) (0 == symcmp((Sym){.ptr= __cstr, .len= strlen(__cstr)}, __t))

static Obj* underscore_var = NULL;

Obj* _parse_expr(Pars* self, Scope* scope, bool atomic) {
    if (!_lex(self)) return NULL;
    Obj* r = NULL;

    if (tok_is_fun(self->t)) {
        r = scope_get(&exts_scope, self->t);
        if (!r) return NULL;

        if (atomic) return r;

        sz before = self->i;
        if (!_lex(self)) return r;
        if (tok_is(",", self->t)
                || tok_is(")", self->t)
                || tok_is("}", self->t)
                || tok_is(";", self->t))
            self->i = before;

        else {
            u8 argc = 0;
            Obj* argv[64]; // YYY: we'll say that's enough

            do {
                Obj* arg = _parse_expr(self, scope, true);
                if (!arg) return NULL;
                argv[argc++] = arg;
                if (64 < argc) return NULL; // meh
            } while (!tok_is(",", self->t) && !tok_is(";", self->t));

            r = obj_call(r, argc, argv);
            if (!r) return NULL;
        }
    }

    else if (tok_is("(", self->t)) {
        r = _parse_expr(self, scope, false);
        if (!tok_is(")", self->t)) return NULL;
    }

    else if (tok_is_str(self->t)) {
        // TODO: parse escapes
        // TODO: copy to allocated
        // TODO: make obj
        return NULL;
    }

    else if (tok_is_num(self->t)) {
        // TODO: parse number
        // TODO: make obj
        return NULL;
    }

    else if (tok_is("{", self->t)) {
        // TODO: parse list
        // TODO: to allocated
        r = NULL;

        do {
            Obj* item = _parse_expr(self, scope, true);
            if (!item) return NULL;
        } while (_lex(self) && tok_is(",", self->t));

        if (!tok_is("}", self->t)) return NULL;
        // TODO: make obj
        return NULL;
    }

    else if (tok_is_sym(self->t)) {
        // TODO: make obj
        return NULL;
    }

    else if (tok_is("_", self->t)) {
        r = underscore_var;
        if (!r) return NULL;
    }

    else if (tok_is_var(self->t)) {
        r = scope_get(scope, self->t);
        if (!r) return NULL;
    }

    if (atomic) return r;

    sz before = self->i;
    if (!_lex(self)) return r;
    if (!tok_is(",", self->t)) {
        self->i = before;
        return r;
    }

    underscore_var = r;
    return _parse_expr(self, scope, false);
}

bool _parse_script(Pars* self, Scope* scope) {
    do {
        if (!_lex(self)) return true;
        if (!tok_is_var(self->t)) return false;
        Sym const key = self->t;

        if (!_lex(self) || !tok_is("=", self->t)) return false;

        Obj* value = _parse_expr(self, scope, false);
        if (!value) return false;

        scope_put(scope, key, value);
    } while (_lex(self) && tok_is(";", self->t));

    return true;
}

bool lang_process(char* script, Scope* scope) {
    (void)scope;

    Pars p = {.s= script, .i= 0 };
    while (_lex(&p)) {
        printf("token <<%.*s>> (a %s)\n",
                (int)p.t.len, p.t.ptr,
                tok_is_str(p.t) ? "str" :
                tok_is_num(p.t) ? "num" :
                tok_is_fun(p.t) ? "fun" :
                tok_is_var(p.t) ? "var" :
                tok_is_sym(p.t) ? "sym" :
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
