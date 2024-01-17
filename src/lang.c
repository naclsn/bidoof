// TODO(whole file): cleanup on failure; whenever parsing fails
//                   it throws away a bunch of allocated stuff

#include "asrtest.h"
#include "exts.h"
#include "lang.h"

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
//     //| <expr> '?' <expr> ':' <expr>
//     | <expr> '[' <expr> ['..' [<expr>]] ']'
// <unop> ::= '+' '-' '!' '~'
// <binop> ::= '+' '-' '*' '/' '%' '//' '**' '==' '!=' '>' '<' '>=' '<=' '<=>' '&' '|' '^' '<<' '>>'
//
// <bind> ::= <fun> <expr> {<expr>}

typedef struct Slice {
    char const* ptr;
    sz len;
} Slice;

static inline Sym slice2sym(Slice const s) {
    Sym r = {0};
    memcpy(r.txt, s.ptr, s.len < 15 ? s.len : 15);
    return r;
}

typedef struct Pars {
    char const* n;
    char const* s;
    sz i;
    Slice t;
    Obj* unnamed;
    bool unnamed_used;
} Pars;

#define fail(__msg) do {  \
    notify(__msg);        \
    return 0;             \
} while (1)

static void _lex(Pars* self) {
#   define AT              (self->s + self->i)
#   define IN(__lo, __hi)  (__lo <= c && c <= __hi)
#   define lex_fail(__msg) do { notify(__msg); goto failed; } while (1)
    char c;
    while (' ' == (c = *AT)
            || '\t' == c
            || '\n' == c
            || '\r' == c
            ) self->i++;

    if ('#' == c) {
        while ('\n' != *AT) self->i++;
        _lex(self);
        return;
    }

    self->t.ptr = AT;
    self->t.len = 0;

    // (when parsing a number)
    unsigned base = 10;
    unsigned digits = 0;

    switch (c) {
        case '(':
            if ('=' == AT[1] || '$' == AT[1]) {
                self->t.len++;
                self->i++;
            }
            // fallthrough
        case ')':
        case '{': case '}':
        case '[': case ']':
        case ',': case ';':
            self->t.len++;
            self->i++;
            return;

        case '<': case '>':
        case '!':
            if ('=' == AT[1]) {
                self->t.len++;
                self->i++;
            }
            // fallthrough
        case '=':
        case '*': case '/':
        case '&': case '|': case '^':
        case '.':
            if ('!' != c && AT[1] == c) {
                self->t.len++;
                self->i++;
            }
            // fallthrough
        case '+':
        case '%':
        case '~':
            self->t.len++;
            self->i++;
            return;

        case '"': {
            char const* end = AT+1;
            while (*end && '"' != *end) if ('\\' == *(end++)) end++;
            if (!*end) lex_fail("missing closing double quote");
            self->t.len = end+1 - AT;
            self->i+= self->t.len;
        } return;

        case '\'': {
            char const* end = AT;
            do end = strchr(end+1, '\'');
            while (end && '\\' == end[-1]);
            if (!end) lex_fail("missing closing simple quote");
            self->t.len = end+1 - AT;
            self->i+= self->t.len;
        } return;

        case ':':
            do c = self->s[++self->t.len, ++self->i];
            while ('_' == c || IN('0', '9') || IN('A', 'Z') || IN('a', 'z'));
            if (1 == self->t.len) lex_fail("expected symbol name");
            return;

        case '\0':
            return;

        case '-':
            self->t.len++;
            self->i++;
            c = *AT;
            if ('.' == c) lex_fail("expected digits");
            if (!IN('0', '9')) return;
            // fall through
        case '0':
            if ('0' == c) {
                self->t.len++;
                self->i++;
                switch (c = *AT) {
                    case 'b': base =  2; c = self->s[++self->t.len, ++self->i]; break;
                    case 'o': base =  8; c = self->s[++self->t.len, ++self->i]; break;
                    case 'x': base = 16; c = self->s[++self->t.len, ++self->i]; break;
                    default:
                        if ('.' == c || '_' == c || IN('0', '9')) { digits++; break; }
                        if (IN('A', 'Z') || IN('a', 'z')) lex_fail("expected digits or spacing");
                        return;
                }
            }
            // fall through
        default:
            if (IN('1', '9') || (self->t.len && ('.' == c || '_' == c))) {
                bool is_digit;
                while ('_' == c
                    || (is_digit = (  2 == base && (IN('0', '1')) )
                                || (  8 == base && (IN('0', '7')) )
                                || ( 10 == base && (IN('0', '9')) )
                                || ( 16 == base && (IN('0', '9') || IN('A', 'F') || IN('a', 'f')) )
                        )) {
                    c = self->s[++self->t.len, ++self->i];
                    digits+= is_digit;
                }
                if (10 == base && '.' == c) {
                    unsigned ddigits = 0;
                    if ('.' == AT[0] && '.' == AT[1]) return; // eg. `[0..]`
                    do {
                        c = self->s[++self->t.len, ++self->i];
                        is_digit = IN('0', '9');
                        ddigits+= is_digit;
                    } while ('_' == c || is_digit);
                    if (!ddigits) lex_fail("expected decimal digits");
                }
                if (!digits) lex_fail("expected digits");
                // better report as syntax error, ~~even tho~~
                // **because** it could be valid
                if (IN('2', '9') || IN('A', 'Z') || IN('a', 'z')) lex_fail("unexpected characters");
                return;
            }
            if (IN('A', 'Z')) {
                do c = self->s[++self->t.len, ++self->i];
                while (IN('0', '9') || IN('A', 'Z') || IN('a', 'z'));
                return;
            }
            if ('_' == c || IN('a', 'z')) {
                do c = self->s[++self->t.len, ++self->i];
                while ('_' == c || IN('0', '9') || IN('a', 'z'));
                return;
            }
    } // switch c

    {
        char msg[30];
        memcpy(msg, "unexpected character 0x.. (.)", 30);
        unsigned hi = (c & 0xf0) >> 4;
        unsigned lo = (c & 0x0f) >> 0;
        msg[23] = (9 < hi ? 'A'-10 : '0') + hi;
        msg[24] = (9 < lo ? 'A'-10 : '0') + lo;
        if (' ' < c && c <= '~') msg[27] = c;
        else msg[25] = '\0';
        lex_fail(msg);
    }

failed:
    self->t.ptr = AT;
    self->t.len = 0;
    return;

#   undef lex_fail
#   undef IN
#   undef AT
}

static inline bool ar_lex_tokens_fn(void) {
    Pars p = {.s=
        "name Function :symbol _ _ok " //12err"
        "42 -1 +12 - 1 -0.5 0.742 0x2a -one* /"
        "(= ( =) # comment\n"
        "\"str line 1\nstr not-quite-line 2\"\n"
        "},{,]["
    };
    char s[256] = {0};

    if (0 /* generate */) while (_lex(&p), p.t.len) {
        char* h = s;
        for (size_t k = 0; k < p.t.len; k++) switch (p.t.ptr[k]) {
            case '\n': *h++ = '\\'; *h++ = 'n'; break;
            case '"': *h++ = '\\';
            default: *h++ = p.t.ptr[k];
        }
        *h = '\0';
        printf("    else if (_lex(&p), strncmp(\"%s\", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !\"%s\");\n", s, s);
    }

    else if (_lex(&p), strncmp("name", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"name");
    else if (_lex(&p), strncmp("Function", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"Function");
    else if (_lex(&p), strncmp(":symbol", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !":symbol");
    else if (_lex(&p), strncmp("_", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"_");
    else if (_lex(&p), strncmp("_ok", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"_ok");
    else if (_lex(&p), strncmp("42", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"42");
    else if (_lex(&p), strncmp("-1", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"-1");
    else if (_lex(&p), strncmp("+", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"+");
    else if (_lex(&p), strncmp("12", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"12");
    else if (_lex(&p), strncmp("-", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"-");
    else if (_lex(&p), strncmp("1", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"1");
    else if (_lex(&p), strncmp("-0.5", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"-0.5");
    else if (_lex(&p), strncmp("0.742", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"0.742");
    else if (_lex(&p), strncmp("0x2a", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"0x2a");
    else if (_lex(&p), strncmp("-", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"-");
    else if (_lex(&p), strncmp("one", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"one");
    else if (_lex(&p), strncmp("*", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"*");
    else if (_lex(&p), strncmp("/", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"/");
    else if (_lex(&p), strncmp("(=", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"(=");
    else if (_lex(&p), strncmp("(", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"(");
    else if (_lex(&p), strncmp("=", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"=");
    else if (_lex(&p), strncmp(")", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !")");
    else if (_lex(&p), strncmp("\"str line 1\nstr not-quite-line 2\"", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"\"str line 1\nstr not-quite-line 2\"");
    else if (_lex(&p), strncmp("}", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"}");
    else if (_lex(&p), strncmp(",", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !",");
    else if (_lex(&p), strncmp("{", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"{");
    else if (_lex(&p), strncmp(",", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !",");
    else if (_lex(&p), strncmp("]", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"]");
    else if (_lex(&p), strncmp("[", p.t.ptr, p.t.len)) ASSERT_REACH(ar_lex_tokens, !"[");

    else ASSERT_REACH(ar_lex_tokens, (_lex(&p), !p.t.len));
    return true;
}
#define ar_lex_tokens ar_lex_tokens_fn()

void _print_location(Pars* self, char* reason) {
    sz lineNr = 1, colNr = 1;
    sz lineStart = 0, lineEnd = 0;

    for (char const* it = self->s; self->t.ptr != it; it++) {
        colNr++;
        if ('\n' == *it) {
            lineNr++;
            colNr = 1;
            lineStart = (it - self->s) + 1;
        } else if ('\t' == *it) colNr+= 7;
    }

    char* nl = strchr(self->t.ptr, '\n');
    if (nl) lineEnd = nl - self->s;

    char m[1024];

    sprintf(m, "%s %s:%zu:%zu", reason, self->n, lineNr, colNr);
    notify(m);

    if (lineEnd <= lineStart) sprintf(m, "%4zu | %s", lineNr, self->s+lineStart);
    else sprintf(m, "%4zu | %.*s", lineNr, (int)(lineEnd-lineStart), self->s+lineStart);
    notify(m);

    int c = sprintf(m, "%4zu | %*s", lineNr+1, (int)colNr, "^");
    if (self->t.len) {
        for (sz k = 0; k < self->t.len-1; k++) m[c+k] = '~';
        m[c+self->t.len-1] = '\0';
    }
    notify(m);
}

sz _escape(char const* ptr, sz len, u32* res) {
#   define AT_IN(__off, __lo, __hi)  (__lo <= ptr[__off] && ptr[__off] <= __hi)
    *res = 0;

    switch (*ptr) {
        case 'a': *res = 0x07; return 1;
        case 'b': *res = 0x08; return 1;
        case 'e': *res = 0x1B; return 1;
        case 'f': *res = 0x0C; return 1;
        case 'n': *res = 0x0A; return 1;
        case 'r': *res = 0x0D; return 1;
        case 't': *res = 0x09; return 1;
        case 'v': *res = 0x0B; return 1;
        case '\\': *res = 0x5C; return 1;
        case '\'': *res = 0x27; return 1;
        case '"': *res = 0x22; return 1;

        case 'x': // 2 hex digits byte
            if (len <3
                || !( AT_IN(1, '0', '9') || AT_IN(1, 'A', 'F') || AT_IN(1, 'a', 'f') )
                || !( AT_IN(2, '0', '9') || AT_IN(2, 'A', 'F') || AT_IN(2, 'a', 'f') )
                ) return 0;
            else {
                u8 hi = 32 | ptr[1];
                u8 lo = 32 | ptr[2];
                *res = ((lo & 0xf) + ('9'<lo)*9) | ( ((hi & 0xf) + ('9'<hi)*9) << 4 );
            }
            ASSERT_REACH(ar_escape_x, 42 == *res);
            #define ar_escape_x { u32 res; _escape("x2a", 3, &res); }
            return 3;

        case 'u': // 4 hex digits codepoint below 0x10000
            if (len < 5) return 0;
            for (sz k = 1; k < 5; k++) {
                if (!( AT_IN(k, '0', '9') || AT_IN(k, 'A', 'F') || AT_IN(k, 'a', 'f') )) return 0;
                u8 it = 32 | ptr[k];
                *res = (*res << 4) | ((it & 0xf) + ('9'<it)*9);
            }
            ASSERT_REACH(ar_escape_u, 12295 == *res);
            #define ar_escape_u { u32 res; _escape("u3007", 5, &res); }
            return 5;

        case 'U': // 8 hex digits codepoint
            if (len < 9) return 0;
            for (sz k = 1; k < 9; k++) {
                if (!( AT_IN(k, '0', '9') || AT_IN(k, 'A', 'F') || AT_IN(k, 'a', 'f') )) return 0;
                u8 it = 32 | ptr[k];
                *res = (*res << 4) | ((it & 0xf) + ('9'<it)*9);
            }
            ASSERT_REACH(ar_escape_U, 128027 == *res);
            #define ar_escape_U { u32 res; _escape("U0001f41b", 9, &res); }
            return 9;

        default: // 3 oct digits byte
            if ('0' <= *ptr && *ptr <= '7') {
                if (len <3
                        || !AT_IN(0, '0', '7')
                        || !AT_IN(1, '0', '7')
                        || !AT_IN(2, '0', '7')
                   ) return 0;
                else {
                    u8 hi = ptr[0] & 0xf;
                    u8 mi = ptr[1] & 0xf;
                    u8 lo = ptr[2] & 0xf;
                    *res = lo | (mi <<3) | (hi <<6);
                }
                ASSERT_REACH(ar_escape_o, 0644 == *res);
                #define ar_escape_o { u32 res; _escape("644", 9, &res); }
                return 3;
            }
    }

    return 0;
#   undef AT_IN
}

static bool _update_free_str(Obj* self) {
    if (!self->update) free(self->as.buf.ptr);
    return true;
}

static bool _update_free_lst(Obj* self) {
    if (!self->update) {
        for (sz k = 0; k < self->as.lst.len; k++) {
            Obj* it = self->as.lst.ptr[k];
            if (!--it->keepalive) {
                obj_destroy(it);
                free(it);
            }
        }
        free(self->as.lst.ptr);
    }
    return true;
}

static Obj* _parse_expr(Pars* self, Scope* scope, bool atomic);

#define tok_is_str(__t) ('"' == (__t).ptr[0])
#define tok_is_num(__t) ('\''== (__t).ptr[0] || ('0' <= (__t).ptr[0] && (__t).ptr[0] <= '9') || ('-' == (__t).ptr[0] && 1 < (__t).len))
#define tok_is_fun(__t) ('A' <= (__t).ptr[0] && (__t).ptr[0] <= 'Z')
#define tok_is_sym(__t) (':' == (__t).ptr[0])
#define tok_is_var(__t) ('_' == (__t).ptr[0] || ('a' <= (__t).ptr[0] && (__t).ptr[0] <= 'z'))
#define tok_is(__cstr, __t) (strlen(__cstr) == (__t).len && 0 == strncmp(__cstr, (__t).ptr, (__t).len))

#define tok_is_unop(__t) (tok_is("+", __t) || tok_is("-", __t) || tok_is("!", __t) || tok_is("~", __t))
#define tok_is_binop(__t) (tok_is("+", __t) || tok_is("-", __t) || tok_is("*", __t) || tok_is("/", __t) || tok_is("**", __t))

#define tok_is_end(__t) (tok_is(",", self->t) || tok_is(")", self->t) || tok_is("}", self->t) || tok_is(";", self->t) || tok_is("", self->t) || tok_is_binop(self->t))

static Obj* _math_unary(Slice const op) {
    char const* name = "OopsyUnary";
    switch (op.ptr[0]) {
        case '+': name = "Id"; break;
        case '-': name = "Negate"; break;
        case '!': name = "BoolNegate"; break;
        case '~': name = "BinNegate"; break;
        //case '|': "_" || "^" ..
    }
    Obj* r = scope_get(&exts_scope, mksym(name));
    if (!r) fail("NIY: function for this unary operator");
    return r;
}
static Obj* _math_binary(Slice const op) {
    char const* name = "OopsyBinary";
    switch (op.ptr[0]) {
        case '+': name = "Add"; break;
        case '-': name = "Sub"; break;
        case '*': name = 1 == op.len ? "Mul" : "Pow"; break;
        case '/': name = "Div"; break;
    }
    Obj* r = scope_get(&exts_scope, mksym(name));
    if (!r) fail("NIY: function for this binary operator");
    return r;
}

short _precedence(Slice const op) {
    switch (op.ptr[0]) {
        case '+': case '-': return 11;
        case '*': return 1 == op.len ? 12 : 13;
        case '/': return 12;
    }
    //printf("op: <<%.*s>>\n", (int)op.len, op.ptr);
    return 99;
}

static Obj* _parse_expr_math_1(Pars* self, Scope* scope) {
    if (tok_is_unop(self->t)) {
        Obj* f = _math_unary(self->t);

        _lex(self);
        Obj* arg = _parse_expr_math_1(self, scope);
        if (!arg) fail("in operand for unary operator");

        Obj* r = calloc(1, sizeof(Obj));
        if (!r) fail("OOM");

        dyarr_zero(&r->args);
        *dyarr_push(&r->args) = arg;

        if (!obj_call(f, r)) {
            free(r);
            fail("could not call function for unary operator");
        }

        return r;
    }

    Obj* r = _parse_expr(self, scope, false);
    if (!r) return NULL;

    if (tok_is("[", self->t)) {
        _lex(self);

        Obj* st = _parse_expr_math_1(self, scope);
        Obj* ed = NULL;
        if (!st) fail("in indexing operator");

        Obj* rr;
        Obj* ff;

        if (tok_is("..", self->t)) {
            _lex(self);
            ff = scope_get(&exts_scope, mksym("Slice"));
            if (!tok_is("]", self->t)) {
                ed = _parse_expr_math_1(self, scope);
                if (!ed) fail("in indexing operator end");
            }
        } else ff = scope_get(&exts_scope, mksym("At"));
        if (!tok_is("]", self->t)) fail("missing closing bracket");

        rr = malloc(sizeof(Obj));
        if (!rr) fail("OOM");

        dyarr_zero(&rr->args);
        *dyarr_push(&rr->args) = r;
        *dyarr_push(&rr->args) = st;
        if (ed) *dyarr_push(&rr->args) = ed;

        if (!obj_call(ff, rr)) {
            fail("could not call function for indexing operator");
        }

        _lex(self);

        r = rr;
    }

    return r;
} // _parse_math_expr

static Obj* _parse_expr_math_2(Pars* self, Scope* scope, Obj* lhs, Slice const op) {
    Obj* rhs = _parse_expr_math_1(self, scope);
    Slice nop = self->t;

    bool tail = tok_is(")", nop);
    // if tail then:  left [lastOp] right ")"

    if (!tail) {
        bool isbinop = tok_is_binop(self->t);
        // goal is to identify eg. `(= 1-2-3)` and break the <<-3>> token in two
        bool isnnum = !isbinop && '-' == *self->t.ptr;
        if (!isbinop && !isnnum) fail("expected binary operator");
        if (isnnum) {
            nop.len = 1;
            self->t.ptr++;
            self->t.len--;
        } else _lex(self);
    }

    bool nfirst = !tail && _precedence(op) < _precedence(nop);
    // if nfirst
    //     then:  left [lastOp] (right [nextOp] ...)
    //     else:  (left [lastOp] right) [nextOp] ...

    if (!tail && nfirst) {
        rhs = _parse_expr_math_2(self, scope, rhs, nop);
        if (!rhs) fail("in rhs");
    }

    if (FLT == lhs->ty || FLT == rhs->ty) {
        // TODO: promotion to Flt
    }

    Obj* f = _math_binary(op);
    Obj* r = malloc(sizeof(Obj));
    if (!r) fail("OOM");

    dyarr_zero(&r->args);
    *dyarr_push(&r->args) = lhs;
    *dyarr_push(&r->args) = rhs;

    if (!obj_call(f, r)) {
        free(r);
        fail("could not call function for binary operator");
    }

    if (!tail && !nfirst) {
        r = _parse_expr_math_2(self, scope, r, nop);
        if (!r) fail("in lhs");
    }

    return r;
} // _parse_math

static Obj* _parse_expr_fun(Pars* self, Scope* scope, bool atomic) {
    Slice const funct = self->t;

    Obj* f = scope_get(&exts_scope, slice2sym(self->t));
    if (!f) fail("unknown function");

    _lex(self);
    if (atomic) return f;

    Obj* r = calloc(1, sizeof(Obj));
    if (!r) fail("OOM");

    dyarr(Obj*) args = {0};
    while (!tok_is_end(self->t)) {
        Obj** arg = dyarr_push(&args);
        if (!arg) {
            notify("OOM");
            goto failed;
        }

        *arg = _parse_expr(self, scope, true);
        if (!*arg) {
            notify("in function argument");
            goto failed;
        }
    }

    r->args.ptr = args.ptr;
    r->args.len = args.len;
    r->args.cap = args.cap;

    if (!obj_call(f, r)) {
        //obj_destroy(r); probly not, obj_call should make sure there is nothing to cleanup if it fails
        self->t = funct; // on function being called, for error message
        notify("could not call function");
        goto failed;
    }

    // TODO: the result effectively depends on f (the function)
    //       when you eventually come back to it, also do make argv a dyarr, thanks
    return r;

failed:
    for (size_t k = 0; k < args.len; k++) {
        // XXX: free args.ptr[k]
    }
    free(args.ptr);
    free(r);
    return NULL;
} // _parse_expr_fun

static Obj* _parse_expr_sub(Pars* self, Scope* scope) {
    Slice const open = self->t;

    // `x, (a _, b _), y _`  this is not valid, `_` in `a _` doesn't exist
    // `x, a _, b _, y _`  this is the correct way to write it

    Obj* punnamed = self->unnamed;
    bool punnamed_used = self->unnamed_used;
    self->unnamed = NULL;
    self->unnamed_used = false;

    _lex(self);
    Obj* r = _parse_expr(self, scope, false);

    self->unnamed = punnamed;
    self->unnamed_used = punnamed_used;
    if (!r) fail("in parenthesised expression");

    if (!tok_is(")", self->t)) {
        self->t = open; // on matching openning, for error message
        fail("missing closing parenthesis");
    }

    _lex(self);
    return r;
}

// TODO: test and finish fixing (this and the functions it depends on)
static Obj* _parse_expr_math(Pars* self, Scope* scope) {
    Obj* r = NULL;
    Slice const open = self->t;

    _lex(self);
    Obj* first = _parse_expr_math_1(self, scope);
    if (!first) fail("in math expression lhs");

    bool isbinop = tok_is_binop(self->t);
    // goal is to identify eg. `(= 1-2)` and break the <<-2>> token in two
    bool isnnum = !isbinop && '-' == *self->t.ptr;
    if (isbinop || isnnum) {
        Slice op = self->t;
        if (isnnum) {
            op.len = 1;
            self->t.ptr++;
            self->t.len--;
        } else _lex(self);

        r = _parse_expr_math_2(self, scope, first, op);
        if (!r) {
            // XXX: free first
            fail("in math expression rhs");
        }
    } else r = first;

    if (!tok_is(")", self->t)) {
        self->t = open; // on matching openning, for error message
        // XXX: free r
        fail("missing closing parenthesis");
    }

    _lex(self);
    return r;
}

static Obj* _parse_expr_bind(Pars* self, Scope* scope) {
    (void)scope;
    fail("NIY: bind shorthand syntax (`Bind` itself isn't implemented either anyways...)");

    Slice const open = self->t;
    _lex(self);

    // TODO

    if (!tok_is(")", self->t)) {
        self->t = open;
        fail("missing closing parenthesis");
    }
    _lex(self);
}

static Obj* _parse_expr_str(Pars* self) {
    // we don't try to be tight..
    // (ie. this will waste the space of any escape sequence)
    u8* bufptr = malloc(self->t.len-2);
    sz buflen = 0;
    if (!bufptr) fail("OOM");

    for (sz k = 1; k < self->t.len-1; k++) {
        if ('\\' != self->t.ptr[k])
            bufptr[buflen++] = self->t.ptr[k];
        else {
            u32 val = 0;
            k++;
            sz sk = _escape(self->t.ptr+k, self->t.len-1-k, &val);
            if (0 == sk) {
                free(bufptr);
                fail("invalid escape sequence");
            }
            k+= sk-1;

            if (1 == sk || 3 == sk)
                bufptr[buflen++] = val & 0xff;
            else {
                // unicode to utf8
                if (val < 128) bufptr[buflen++] = val;
                else {
                    u8 x = val & 63;
                    val>>= 6;
                    if (val < 32) bufptr[buflen++] = 192 | val;
                    else {
                        u8 y = val & 63;
                        val>>= 6;
                        if (val < 16) bufptr[buflen++] = 224 | val;
                        else {
                            u8 z = val & 63;
                            bufptr[buflen++] = 240 | (val >> 6);
                            bufptr[buflen++] = 128 | z;
                        }
                        bufptr[buflen++] = 128 | y;
                    }
                    bufptr[buflen++] = 128 | x;
                }
            } // if unicode
        } // if '\\'
    } // for chars in literal

    ASSERT_REACH(ar_parse_expr_str, !strncmp("hi :3 \\", bufptr, buflen));
    #define ar_parse_expr_str Pars p = {.s= "\"hi :\x33 \\\\\""}; _lex(&p); _parse_expr_str(&p);

    Obj* r = calloc(1, sizeof(Obj));
    if (!r) {
        free(bufptr);
        fail("OOM");
    }
    r->ty = BUF;
    r->as.buf.ptr = bufptr;
    r->as.buf.len = buflen;
    r->update = _update_free_str;

    _lex(self);
    return r;
} // _parse_expr_str

static Obj* _parse_expr_num(Pars* self) {
    i64 ival;
    f64 fval;
    bool negative = '-' == self->t.ptr[0];
    bool floating = false;

    if ('\'' == self->t.ptr[0]) {
        u32 val = self->t.ptr[1];
        if ('\\' == val && 0 == _escape(self->t.ptr+2, self->t.len-2, &val))
            fail("invalid escape sequence");
        ival = val;
    } else {
        sz k = negative;
        ival = 0;
        if ('0' == self->t.ptr[k] && 1 < self->t.len) switch (self->t.ptr[k+1]) {
            case 'b':
                for (k+= 2; k < self->t.len; k++) if ('_' != self->t.ptr[k])
                    ival = (ival << 1) | (self->t.ptr[k] & 0xf);
                break;

            case 'o':
                for (k+= 2; k < self->t.len; k++) if ('_' != self->t.ptr[k])
                    ival = (ival <<3 ) | (self->t.ptr[k] & 0xf);
                break;

            case 'x':
                for (k+= 2; k < self->t.len; k++) if ('_' != self->t.ptr[k]) {
                    u8 it = 32 | self->t.ptr[k];
                    ival = (ival << 4) | ((it & 0xf) + ('9'<it)*9);
                }
                ASSERT_REACH(ar_parse_expr_num_1, 42 == ival);
                #define ar_parse_expr_num_1 Pars p = {.s= "-0x2a"}; _lex(&p); _parse_expr_num(&p);
                break;

            default: goto decimal;
        } else {
        decimal:

            for (; k < self->t.len && '.' != self->t.ptr[k]; k++) if ('_' != self->t.ptr[k])
                ival = ival*10 + (self->t.ptr[k] & 0xf);

            if (k < self->t.len && '.' == self->t.ptr[k]) {
                floating = true;
                fval = 0;
                for (sz f = self->t.len-1; f > k; f--) if ('_' != self->t.ptr[f])
                    fval = fval/10 + (self->t.ptr[f] & 0xf);
                fval = ival + fval/10;
            }

            ASSERT_REACH(ar_parse_expr_num_2, 123.456 == fval);
            #define ar_parse_expr_num_2 Pars p = {.s= "1_2_3.4_5_6"}; _lex(&p); _parse_expr_num(&p);
        } // if (not) 0[box..]
    } // if (not) '\''

    Obj* r = calloc(1, sizeof(Obj));
    if (!r) fail("OOM");
    if (floating) {
        r->ty = FLT;
        r->as.flt.val = negative ? -fval : fval;
    } else {
        r->ty = NUM;
        r->as.num.val = negative ? -ival : ival;
    }

    _lex(self);
    return r;
} // _parse_expr_num

static Obj* _parse_expr_lst(Pars* self, Scope* scope) {
    Slice const open = self->t;

    Obj* r = calloc(1, sizeof(Obj));
    if (!r) fail("OOM");

    dyarr(Obj*) items = {0};
    do {
        Obj** item = dyarr_push(&items);
        if (!item) {
            notify("OOM");
            goto failed;
        }

        _lex(self);
        *item = _parse_expr(self, scope, true);
        if (!*item) {
            notify("in list expression");
            goto failed;
        }

        // TODO: change to the list being a proper depnt
        // which means that when an item gets updated,
        // the whole list does too
        (*item)->keepalive++;
    } while (tok_is(",", self->t));

    if (!tok_is("}", self->t)) {
        self->t = open; // on matching openning, for error message
        notify("missing closing brace");
        goto failed;
    }

    r->ty = LST;
    r->as.lst.ptr = items.ptr;
    r->as.lst.len = items.len;
    r->update = _update_free_lst;

    _lex(self);
    return r;

failed:
    for (size_t k = 0; k < items.len; k++) {
        // XXX: free items.ptr[k]
    }
    free(items.ptr);
    free(r);
    return NULL;
} // _parse_expr_lst

static Obj* _parse_expr_sym(Pars* self) {
    Obj* r = calloc(1, sizeof(Obj));
    if (!r) fail("OOM");
    r->ty = SYM;
    r->as.sym = slice2sym((++self->t.ptr, --self->t.len, self->t));

    _lex(self);
    return r;
}

static Obj* _parse_expr_unnamed(Pars* self) {
    Obj* r = self->unnamed;
    self->unnamed_used = true;
    if (!r) fail("no unnamed variable at this point");

    _lex(self);
    return r;
}

static Obj* _parse_expr_var(Pars* self, Scope* scope) {
    Obj* r = scope_get(scope, slice2sym(self->t));
    if (!r) fail("unknown variable");

    _lex(self);
    return r;
}

static Obj* _parse_expr(Pars* self, Scope* scope, bool atomic) {
    Obj* r = NULL;

    if (0);
    else if (tok_is_fun(self->t))   r = _parse_expr_fun(self, scope, atomic);
    else if (tok_is("(", self->t))  r = _parse_expr_sub(self, scope);
    else if (tok_is("(=", self->t)) r = _parse_expr_math(self, scope);
    else if (tok_is("($", self->t)) r = _parse_expr_bind(self, scope);
    else if (tok_is_str(self->t))   r = _parse_expr_str(self);
    else if (tok_is_num(self->t))   r = _parse_expr_num(self);
    else if (tok_is("{", self->t))  r = _parse_expr_lst(self, scope);
    else if (tok_is_sym(self->t))   r = _parse_expr_sym(self);
    else if (tok_is("_", self->t))  r = _parse_expr_unnamed(self);
    else if (tok_is_var(self->t))   r = _parse_expr_var(self, scope);
    else fail("expected expression");

    if (atomic) return r;

    // NOTE: this could, and probly should, be re-written as to not be recursive
    if (r && tok_is(",", self->t)) {
        Obj* punnamed = self->unnamed;
        bool punnamed_used = self->unnamed_used;
        self->unnamed = r;
        self->unnamed_used = false;
#ifdef ASR_TEST_BUILD
        Obj const* const pr = r;
#endif

        _lex(self);
        r = _parse_expr(self, scope, false);

        ASSERT_REACH(ar_parse_expr_unnamed_unused, !self->unnamed_used);
        #define ar_parse_expr_unnamed_unused Pars p = {.s= "Fn1 a, Fn2 b c"}; _lex(&p); _parse_expr(&p, NULL, false);
        ASSERT_REACH(ar_parse_expr_unnamed_used, self->unnamed_used);
        #define ar_parse_expr_unnamed_used Pars p = {.s= "Fn1 a, Fn2 _ c"}; _lex(&p); _parse_expr(&p, NULL, false);
        // this causes a double test :/
        ASSERT_REACH(ar_parse_expr_unnamed_restored, pr == self->unnamed);
        #define ar_parse_expr_unnamed_restored Pars p = {.s= "Fn1 a, Fn2 _ c, Fn1 _"}; _lex(&p); _parse_expr(&p, NULL, false);
        if (!self->unnamed_used) {
            // XXX: free self->unnamed
        }

        self->unnamed = punnamed;
        self->unnamed_used = punnamed_used;
    }

    return r;
} // _parse_expr

static bool _parse_script(Pars* self, Scope* scope) {
    while (!tok_is("", self->t)) {
        if (!tok_is_var(self->t)) fail("expected variable name");
        Slice const name = self->t;
        _lex(self);

        if (!tok_is("=", self->t)) fail("expected equal");
        _lex(self);

        Obj* value = _parse_expr(self, scope, false);
        if (!value) fail("in script expression");

        Sym const key = slice2sym(name);
        if (!scope_put(scope, key, value)) {
            // XXX: free value
            fail("OOM");
        }

        if (!tok_is(";", self->t)) break;
        _lex(self);
    }

    if (!tok_is("", self->t)) fail("unexpected token");

    return true;
}

bool lang_process(char const* name, char const* script, Scope* scope) {
    Pars p = {.n= name, .s= script, .i= 0};

    if (_lex(&p), _parse_script(&p, scope)) return true;

    _print_location(&p, "ERROR");
    return false;
}

void lang_show_tokens(char const* name, char const* script) {
    Pars p = {.n= name, .s= script, .i= 0};
    while (_lex(&p), !tok_is("", p.t)) {
        char m[21+p.t.len];
        sprintf(m, "token <<%.*s>> (a %s)",
                (int)p.t.len, p.t.ptr,
                tok_is_str(p.t) ? "str" :
                tok_is_num(p.t) ? "num" :
                tok_is_fun(p.t) ? "fun" :
                tok_is_var(p.t) ? "var" :
                tok_is_sym(p.t) ? "sym" :
                "punct");
        notify(m);
        _print_location(&p, "TOKEN");
        notify("");
    }

    if (!*p.t.ptr) notify("end of script");
    else _print_location(&p, "ERROR");
}

#ifdef ASR_TEST_BUILD
Scope exts_scope = {0};
bool _Fn_call(Obj* self, Obj* res) { return *res = (Obj){0}, true; }
Obj* scope_get(Scope* self, Sym const key) {
    if (&exts_scope == self) {
        if (!strcmp("Fn1", key.txt)) {
            static Obj f1 = {.ty= FUN, .as.fun.call= _Fn_call};
            return &f1;
        }
        if (!strcmp("Fn2", key.txt)) {
            static Obj f2 = {.ty= FUN, .as.fun.call= _Fn_call};
            return &f2;
        }
    } else {
        if ('\0' == key.txt[1]) switch (key.txt[0]) {
            case 'a': static Obj a = {.ty= NUM, .as.num.val= 1}; return &a; break;
            case 'b': static Obj b = {.ty= NUM, .as.num.val= 2}; return &b; break;
            case 'c': static Obj c = {.ty= NUM, .as.num.val= 3}; return &c; break;
            case 'd': static Obj d = {.ty= NUM, .as.num.val= 4}; return &d; break;
        }
    }
    printf("scope_get <<%s>> => NULL\n", key.txt);
    return NULL;
}
bool scope_put(Scope* self, Sym const key, Obj* value) { return true; }

bool obj_call(Obj* self, Obj* res) { return true; }
void obj_destroy(Obj* self) { }

void notify_default(char const* s) { printf("!! %s\n", s); }
void (*notify)(char const* s) = notify_default;
#endif // ASR_TEST_BUILD

ASR_MAIN(
    ASR_TEST(ar_lex_tokens);
    ASR_TEST(ar_escape_x);
    ASR_TEST(ar_escape_u);
    ASR_TEST(ar_escape_U);
    ASR_TEST(ar_escape_o);
    ASR_TEST(ar_parse_expr_str);
    ASR_TEST(ar_parse_expr_num_1);
    ASR_TEST(ar_parse_expr_num_2);
    ASR_TEST(ar_parse_expr_unnamed_unused);
    ASR_TEST(ar_parse_expr_unnamed_used);
    ASR_TEST(ar_parse_expr_unnamed_restored);
)
