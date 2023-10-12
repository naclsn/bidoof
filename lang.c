// TODO(whole file): cleanup on failure; whenever parsing fails
//                   it throws away a bunch of allocated stuff

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
//     | <expr> '?' <expr> ':' <expr>
//     | <expr> '[' <expr> [':' [<expr>]] ']'
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
} Pars;

#define fail(__msg) do {  \
    notify(__msg);        \
    return 0;             \
} while (1)

bool _lex(Pars* self) {
#define AT              (self->s + self->i)
#define IN(__lo, __hi)  (__lo <= c && c <= __hi)
    char c;
    while (' ' == (c = *AT)
            || '\t' == c
            || '\n' == c
            || '\r' == c
            ) self->i++;

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
        case ',': case ';':
            self->t.len++;
            self->i++;
            return true;

        case '<': case '>':
        case '!':
            if (AT[1] == '=') {
                self->t.len++;
                self->i++;
            }
            // fallthrough
        case '=':
        case '*': case '/':
        case '&': case '|': case '^':
            if ('!' != c && AT[1] == c) {
                self->t.len++;
                self->i++;
            }
            // fallthrough
        case '+': case '-':
        case '%':
        case '~':
        case '#':
            self->t.len++;
            self->i++;
            return true;


        case '"': {
            char const* end = AT+1;
            while (*end && '"' != *end) if ('\\' == *(end++)) end++;
            if (!*end) fail("missing closing double quote");
            self->t.len = end+1 - AT;
            self->i+= self->t.len;
        } return true;

        case '0':
            if ('0' == c) {
                self->t.len++;
                self->i++;
                switch (c = *AT) {
                    case 'b': base =  2; c = self->s[++self->t.len, ++self->i]; break;
                    case 'o': base =  8; c = self->s[++self->t.len, ++self->i]; break;
                    case 'x': base = 16; c = self->s[++self->t.len, ++self->i]; break;
                    case '.': case '_':
                    case '0'...'9': digits++; break;
                    default:
                        if (IN('A', 'Z') || IN('a', 'z')) fail("expected digits or spacing");
                        return true;
                }
            }
            // fall through
        case '1'...'9': {
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
                do {
                    c = self->s[++self->t.len, ++self->i];
                    is_digit = IN('0', '9');
                    ddigits+= is_digit;
                } while ('_' == c || is_digit);
                if (!ddigits) fail("expected decimal digits");
            }
            if (!digits) fail("expected digits");
            // better report as syntax error, ~~even tho~~
            // **because** it could be valid
            if (IN('2', '9') || IN('A', 'Z') || IN('a', 'z')) fail("unexpected characters");
        } return true;

        case '\'': {
            char const* end = AT;
            do end = strchr(end+1, '\'');
            while (end && '\\' == end[-1]);
            if (!end) fail("missing closing simple quote");
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
            if (1 == self->t.len) fail("expected symbol name");
            return true;

        case '_':
        case 'a'...'z':
            do c = self->s[++self->t.len, ++self->i];
            while ('_' == c || IN('0', '9') || IN('a', 'z'));
            return true;

        case '\0':
            return true;
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
        fail(msg);
    }
#undef IN
#undef AT
}

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
#define AT_IN(__off, __lo, __hi)  (__lo <= ptr[__off] && ptr[__off] <= __hi)
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
                u8 hi = 0b100000 | ptr[1];
                u8 lo = 0b100000 | ptr[2];
                *res = ((lo & 0xf) + ('9'<lo)*9) | ( ((hi & 0xf) + ('9'<hi)*9) << 4 );
            }
            return 3;

        case 'u': // 4 hex digits codepoint below 0x10000
            if (len < 5) return 0;
            for (sz k = 1; k < 5; k++) {
                if (!( AT_IN(k, '0', '9') || AT_IN(k, 'A', 'F') || AT_IN(k, 'a', 'f') )) return 0;
                u8 it = 0b100000 | ptr[k];
                *res = (*res << 4) | ((it & 0xf) + ('9'<it)*9);
            }
            return 5;

        case 'U': // 8 hex digits codepoint
            if (len < 9) return 0;
            for (sz k = 1; k < 9; k++) {
                if (!( AT_IN(k, '0', '9') || AT_IN(k, 'A', 'F') || AT_IN(k, 'a', 'f') )) return 0;
                u8 it = 0b100000 | ptr[k];
                *res = (*res << 4) | ((it & 0xf) + ('9'<it)*9);
            }
            return 9;

        case '0'...'7': // 3 oct digits byte
            if (len <3
                || !AT_IN(0, '0', '7')
                || !AT_IN(1, '0', '7')
                || !AT_IN(2, '0', '7')
                ) return 0;
            else {
                u8 hi = ptr[0] & 0xf;
                u8 mi = ptr[1] & 0xf;
                u8 lo = ptr[2] & 0xf;
                *res = lo | (mi <<3) | (hi <<3);
            }
            return 3;
    }

    return 0;
#undef AT_IN
}

bool _update_free_str(Obj* self) {
    if (!self->update) free(self->as.buf.ptr);
    return true;
}

bool _update_free_lst(Obj* self) {
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

Obj* _parse_expr(Pars* self, Scope* scope, bool atomic);

#define tok_is_str(__t) ('"' == (__t).ptr[0])
#define tok_is_num(__t) ('\''== (__t).ptr[0] || ('0' <= (__t).ptr[0] && (__t).ptr[0] <= '9'))
#define tok_is_fun(__t) ('A' <= (__t).ptr[0] && (__t).ptr[0] <= 'Z')
#define tok_is_sym(__t) (':' == (__t).ptr[0])
#define tok_is_var(__t) ('_' == (__t).ptr[0] || ('a' <= (__t).ptr[0] && (__t).ptr[0] <= 'z'))
#define tok_is(__cstr, __t) (strlen(__cstr) == (__t).len && 0 == strncmp(__cstr, (__t).ptr, (__t).len))

#define tok_is_unop(__t) (tok_is("+", __t) || tok_is("-", __t) || tok_is("!", __t) || tok_is("~", __t))
#define tok_is_binop(__t) (tok_is("+", __t) || tok_is("-", __t) || tok_is("*", __t) || tok_is("/", __t) || tok_is("**", __t))

#define tok_is_end(__t) (tok_is(",", self->t) || tok_is(")", self->t) || tok_is("}", self->t) || tok_is(";", self->t) || tok_is("", self->t) || tok_is_binop(self->t))

static inline Obj* _math_unary(Slice const op) {
    char const* name = "OopsyUnary";
    switch (op.ptr[0]) {
        case '+': name = "Posite"; break; // ??
        case '-': name = "Negate"; break;
        case '!': name = "BoolNegate"; break;
        case '~': name = "BinNegate"; break;
        //case '|': "_" || "^" ..
    }
    Obj* r = scope_get(&exts_scope, mksym(name));
    if (!r) fail("NIY: function for this unary operator");
    return r;
}
static inline Obj* _math_binary(Slice const op) {
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
    return 99;
}

Obj* _parse_math_expr(Pars* self, Scope* scope) {
    if (tok_is_unop(self->t)) {
        if (!_lex(self)) fail("expected operand for unary operator");

        Obj* f = _math_unary(self->t);
        Obj* arg = _parse_math_expr(self, scope);
        if (!arg) fail("in operand for unary operator");

        Obj* r = calloc(1, sizeof(Obj) + 1*sizeof(Obj*));
        if (!r) fail("OOM");

        r->argc = 1;
        r->argv[0] = arg;

        if (!obj_call(f, r)) {
            free(r);
            fail("could not call function for unary operator");
        }

        return r;
    }

    return _parse_expr(self, scope, false);
}

Obj* _parse_math(Pars* self, Scope* scope, Obj* lhs, Slice const op) {
    Obj* rhs = _parse_math_expr(self, scope);
    Slice const nop = self->t;

    bool tail = tok_is(")", nop);
    // if tail then:  left [lastOp] right ")"

    if (!tail) {
        if (!tok_is_binop(nop)) fail("expected binary operator");
        if (!_lex(self)) fail("expected operand for binary operator");
    }

    bool nfirst = !tail && _precedence(op) < _precedence(self->t);
    // if nfirst
    //     then:  left [lastOp] (right [nextOp] ...)
    //     else:  (left [lastOp] right) [nextOp] ...

    if (!tail && nfirst) {
        rhs = _parse_math(self, scope, rhs, nop);
        if (!rhs) fail("in rhs");
    }

    Obj* f = _math_binary(op);
    Obj* r = calloc(1, sizeof(Obj) + 2*sizeof(Obj*));
    if (!r) fail("OOM");

    r->argc = 2;
    r->argv[0] = lhs;
    r->argv[1] = rhs;

    if (!obj_call(f, r)) {
        free(r);
        fail("could not call function for binary operator");
    }

    if (!tail && !nfirst) {
        r = _parse_math(self, scope, r, nop);
        if (!r) fail("in lhs");
    }

    return r;
}

Obj* _parse_expr(Pars* self, Scope* scope, bool atomic) {
    Obj* r = NULL;

    if (tok_is_fun(self->t)) {
        Slice const funct = self->t;

        r = scope_get(&exts_scope, slice2sym(self->t));
        if (!r) fail("unknown function");
        _lex(self);

        if (atomic) return r;

        dyarr(Obj*) args = {0};
        while (!tok_is_end(self->t)) {
            Obj** arg = dyarr_push(&args);
            if (!arg) fail("OOM");

            *arg = _parse_expr(self, scope, true);
            if (!*arg) fail("in function argument");
        }

        Obj* rr = calloc(1, sizeof(Obj) + args.len*sizeof(Obj*));
        if (!rr) fail("OOM");

        rr->argc = args.len;
        memcpy(&rr->argv, args.ptr, args.len*sizeof(Obj*));

        if (!obj_call(r, rr)) {
            free(rr);
            self->t = funct;
            fail("could not call function");
        }

        // TODO: shouldn't r (the function) be dropped?
        //       -> no, the result effectively depends on it (TODO)
        //       when you eventually come back to it, also do make argv a dyarr, thanks
        r = rr;
    }

    else if (tok_is("(", self->t)) {
        Slice const open = self->t;
        _lex(self);

        r = _parse_expr(self, scope, false);
        if (!r) fail("in parenthesised expression");

        if (!tok_is(")", self->t)) {
            self->t = open;
            fail("missing closing parenthesis");
        }
        _lex(self);
    }

    else if (tok_is("(=", self->t)) {
        Slice const open = self->t;
        _lex(self);

        Obj* first = _parse_math_expr(self, scope);
        if (!first) fail("in math expression lhs");

        if (tok_is_binop(self->t)) {
            Slice const op = self->t;
            _lex(self);

            r = _parse_math(self, scope, first, op);
            if (!r) fail("in math expression rhs");
        } else r = first;

        if (!tok_is(")", self->t)) {
            self->t = open;
            fail("missing closing parenthesis");
        }
        _lex(self);
    }

    else if (tok_is("($", self->t)) {
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

    else if (tok_is_str(self->t)) {
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
                if (0 == sk) fail("invalid escape sequence");
                k+= sk-1;

                if (1 == sk || 3 == sk)
                    bufptr[buflen++] = val & 0xff;
                else {
                    // unicode to utf8
                    if (val < 0b10000000) bufptr[buflen++] = val;
                    else {
                        u8 x = val & 0b00111111;
                        val>>= 6;
                        if (val < 0b00100000) bufptr[buflen++] = 0b11000000 | val;
                        else {
                            u8 y = val & 0b00111111;
                            val>>= 6;
                            if (val < 0b00010000) bufptr[buflen++] = 0b11100000 | val;
                            else {
                                u8 z = val & 0b00111111;
                                bufptr[buflen++] = 0b11110000 | (val >> 6);
                                bufptr[buflen++] = 0b10000000 | z;
                            }
                            bufptr[buflen++] = 0b10000000 | y;
                        }
                        bufptr[buflen++] = 0b10000000 | x;
                    }
                } // if unicode
            } // if '\\'
        } // for chars in literal

        if (!(r = calloc(1, sizeof *r))) fail("OOM");
        r->ty = BUF;
        r->as.buf.ptr = bufptr;
        r->as.buf.len = buflen;
        r->update = _update_free_str;

        _lex(self);
    }

    else if (tok_is_num(self->t) || tok_is("-", self->t)) {
        i64 ival;
        f64 fval;
        bool negative = '-' == self->t.ptr[0];
        bool floating = false;

        if (negative) _lex(self);

        if ('\'' == self->t.ptr[0]) {
            u32 val = self->t.ptr[1];
            if ('\\' == val && 0 == _escape(self->t.ptr+2, self->t.len-2, &val))
                fail("invalid escape sequence");
            ival = val;
        } else {
            sz k = 0;
            ival = 0;
            if ('0' == self->t.ptr[k]) switch (self->t.ptr[k+1]) {
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
                        u8 it = 0b100000 | self->t.ptr[k];
                        ival = (ival << 4) | ((it & 0xf) + ('9'<it)*9);
                    }
                    break;

                default: goto decimal;
            } else {
            decimal:

                for (; k < self->t.len && '.' != self->t.ptr[k]; k++) if ('_' != self->t.ptr[k])
                    ival = ival*10 + (self->t.ptr[k] & 0xf);

                if ('.' == self->t.ptr[k]) {
                    floating = true;
                    fval = 0;
                    for (sz f = self->t.len-1; f > k; f--) if ('_' != self->t.ptr[f])
                        fval = fval/10 + (self->t.ptr[f] & 0xf);
                    fval = ival + fval/10;
                }
            } // if (not) 0[box..]
        } // if (not) '\''

        if (!(r = calloc(1, sizeof *r))) fail("OOM");
        if (floating) {
            r->ty = FLT;
            r->as.flt.val = negative ? -fval : fval;
        } else {
            r->ty = NUM;
            r->as.num.val = negative ? -ival : ival;
        }

        _lex(self);
    }

    else if (tok_is("{", self->t)) {
        Slice const open = self->t;

        dyarr(Obj*) items = {0};
        do {
            _lex(self);

            Obj** item = dyarr_push(&items);
            if (!item) fail("OOM");

            *item = _parse_expr(self, scope, true);
            if (!*item) fail("in list expression");

            // TODO: change to the list being a proper depnt
            // which means that when an item gets updated,
            // the whole list does too
            (*item)->keepalive++;
        } while (tok_is(",", self->t));

        if (!tok_is("}", self->t)) {
            self->t = open;
            fail("missing closing brace");
        }
        _lex(self);

        if (!(r = calloc(1, sizeof *r))) fail("OOM");
        r->ty = LST;
        r->as.lst.ptr = items.ptr;
        r->as.lst.len = items.len;
        r->update = _update_free_lst;
    }

    else if (tok_is_sym(self->t)) {
        if (!(r = calloc(1, sizeof *r))) fail("OOM");
        r->ty = SYM;
        r->as.sym = slice2sym((++self->t.ptr, --self->t.len, self->t));
        _lex(self);
    }

    else if (tok_is("_", self->t)) {
        r = self->unnamed;
        if (!r) fail("no unnamed variable at this point");
        _lex(self);
    }

    else if (tok_is_var(self->t)) {
        r = scope_get(scope, slice2sym(self->t));
        if (!r) fail("unknown variable");
        _lex(self);
    }

    else fail("expected expression");

    if (atomic) return r;

    if (tok_is(",", self->t)) {
        self->unnamed = r;
        _lex(self);
        return _parse_expr(self, scope, false);
    }

    return r;
} // _parse_expr

bool _parse_script(Pars* self, Scope* scope) {
    while (!tok_is("", self->t)) {
        if (!tok_is_var(self->t)) fail("expected variable name");
        Slice const name = self->t;
        _lex(self);

        if (!tok_is("=", self->t)) fail("expected equal");
        _lex(self);

        Obj* value = _parse_expr(self, scope, false);
        if (!value) fail("in script expression");

        Sym const key = slice2sym(name);
        if (!scope_put(scope, key, value)) fail("OOM");

        if (!tok_is(";", self->t)) break;
        _lex(self);
    }

    return true;
}

bool lang_process(char const* name, char const* script, Scope* scope) {
    Pars p = {.n= name, .s= script, .i= 0};

    if (_lex(&p) && _parse_script(&p, scope)) return true;

    _print_location(&p, "ERROR");
    return false;
}

void lang_show_tokens(char const* name, char const* script) {
    Pars p = {.n= name, .s= script, .i= 0};
    while (_lex(&p)) {
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

/*
#ifdef ASR_TEST_BUILD
Obj* scope_get(Scope* self, Sym const key) { return NULL; }
Obj* obj_call(Obj* self, u8 argc, Obj** argv) { return NULL; }
bool scope_put(Scope* self, Sym const key, Obj* value) { return false; }
Scope exts_scope = {0};
void notify_default(char const* s) { }
void (*notify)(char const* s) = notify_default;
ASR_MAIN {
    ASR_MAIN_RETURN;
}
#endif // ASR_TEST_BUILD
*/
