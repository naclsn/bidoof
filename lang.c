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
// <bind> ::= <fun> {<expr>}

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
            char const* end = AT;
            do end = strchr(end+1, '"');
            while (end && '\\' == end[-1]);
            if (!end) fail("missing closing double-quote");
            self->t.len = end+1 - AT;
            self->i+= self->t.len;
        } return true;

        case '-':
            self->i++;
            if (!*AT) fail("expected digits");
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
                case ' ': case '\t': case '\n': case '\r': case '\0': return true;
                default: fail("expected digits");
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
            if (10 != base && 2 == self->t.len) fail("expected digits");
            // better report as syntax error, ~~even tho~~
            // **because** it could be valid
            if (IN('2', '9') || IN('A', 'Z') || IN('a', 'z')) fail("unexpected characters");
        } return true;

        case '\'': {
            char const* end = AT;
            do end = strchr(end+1, '\'');
            while (end && '\\' == end[-1]);
            if (!end) fail("missing closing simple-quote");
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
            return false;
    }

    fail("unexpected character");
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

    char location_errmsg[256];
    {
        char* head = location_errmsg;
        // XXX: buffer overrun when:
        // - filename too long (TODO: no filename yet tho)
        // - line too long (TODO: should cut to intersting part anyways)
        head+= sprintf(head, "%s %s:%zu:%zu\n", reason, "<script>", lineNr, colNr);
        if (lineEnd <= lineStart) sprintf(head, "%4zu | %s\n", lineNr, self->s+lineStart);
        else head+= sprintf(head, "%4zu | %.*s\n", lineNr, (int)(lineEnd-lineStart), self->s+lineStart);
        head+= sprintf(head, "%4zu | %*s", lineNr+1, (int)colNr, "^");
    }
    notify(location_errmsg);
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
    if (!self->update) {
        free(self->as.buf.ptr);
        self->as.buf.ptr = NULL;
        self->as.buf.len = 0;
    }
    return true;
}

bool _update_free_lst(Obj* self) {
    if (!self->update) {
        free(self->as.lst.ptr);
        self->as.lst.ptr = NULL;
        self->as.lst.len = 0;
    }
    return true;
}

#define tok_is_str(__t) ('"' == (__t).ptr[0])
#define tok_is_num(__t) ('\'' == (__t).ptr[0]  \
            || ('0' <= (__t).ptr[0] && (__t).ptr[0] <= '9')  \
            || ('-' == (__t).ptr[0] && '0' <= (__t).ptr[1] && (__t).ptr[1] <= '9'))
#define tok_is_fun(__t) ('A' <= (__t).ptr[0] && (__t).ptr[0] <= 'Z')
#define tok_is_sym(__t) (':' == (__t).ptr[0])
#define tok_is_var(__t) ('_' == (__t).ptr[0] || ('a' <= (__t).ptr[0] && (__t).ptr[0] <= 'z'))
#define tok_is(__cstr, __t) (0 == strncmp(__cstr, (__t).ptr, (__t).len))

Obj* _parse_expr(Pars* self, Scope* scope, bool atomic) {
    if (!_lex(self)) return NULL;
    Obj* r = NULL;

    if (tok_is_fun(self->t)) {
        r = scope_get(&exts_scope, slice2sym(self->t));
        if (!r) fail("unknown function");

        if (atomic) return r;

        sz before = self->i;
        if (!_lex(self)) return r;
        self->i = before;
        if (    !( tok_is(",", self->t)
                || tok_is(")", self->t)
                || tok_is("}", self->t)
                || tok_is(";", self->t) )  ) {
            u8 argc = 0;
            Obj* argv[64]; // YYY: we'll say that's enough

            do {
                Obj* arg = _parse_expr(self, scope, true);
                // XXX(cleanup): r, argv[..argc]
                if (!arg) fail("in function argument");

                // meh
                if (sizeof argv / sizeof argv[0] == argc)
                    fail("(we don't handle more than so many arguments for now..)");
                argv[argc++] = arg;

                before = self->i;
                if (!_lex(self)) break;
                self->i = before;
            } while (!( tok_is(",", self->t)
                     || tok_is(")", self->t)
                     || tok_is("}", self->t)
                     || tok_is(";", self->t) ));

            Obj* rr = obj_call(r, argc, argv);
            // XXX(cleanup): r, argv[..]
            if (!rr) fail("could not call function with given arguments");
            r = rr;
        }
    }

    else if (tok_is("(", self->t)) {
        sz before = self->i;

        r = _parse_expr(self, scope, false);
        if (!r) fail("in parenthesised expression");

        if (!tok_is(")", self->t)) {
            self->i = before;
            // XXX(cleanup): r
            fail("missing closing parenthesis");
        }
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
                sz sk = _escape(self->t.ptr+2, self->t.len-1, &val);
                // XXX(cleanup): bufptr
                if (0 == sk) fail("invalid escape sequence");
                k+= sk;

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

        // XXX(cleanup): bufptr
        if (!(r = calloc(1, sizeof *r))) fail("OOM");
        r->ty = BUF;
        r->as.buf.ptr = bufptr;
        r->as.buf.len = buflen;
        r->update = _update_free_str;
    }

    else if (tok_is_num(self->t)) {
        u32 val = 0;
        bool negative = '-' == self->t.ptr[0];

        if ('\'' == self->t.ptr[0]) {
            val = self->t.ptr[1];
            if ('\\' == val && 0 == _escape(self->t.ptr+2, self->t.len-2, &val))
                fail("invalid escape sequence");
        } else {
            // TODO: parse number (for realsies)
            for (sz k = negative; k < self->t.len; k++)
                val = val*10 + self->t.ptr[k]-'0';
        }

        if (!(r = calloc(1, sizeof *r))) fail("OOM");
        r->ty = NUM;
        r->as.num.val = negative ? -val : val;
    }

    else if (tok_is("{", self->t)) {
        sz before = self->i;

        sz cap = 16;
        Obj** lstptr = malloc(cap * sizeof(Obj*));
        sz lstlen = 0;
        if (!lstptr) fail("OOM");

        do {
            Obj* item = _parse_expr(self, scope, true);
            // XXX(cleanup): lstptr[..], lstptr
            if (!item) fail("in list literal");

            if (cap == lstlen) {
                cap*= 2;
                Obj** niw = calloc(cap, sizeof(Obj*));
                // XXX(cleanup): lstptr[..], lstptr
                if (!niw) fail("OOM");
                memcpy(niw, lstptr, lstlen * sizeof(Obj*));
                lstptr = niw;
            }

            lstptr[lstlen++] = item;
            item->keepalive++;
        } while (_lex(self) && tok_is(",", self->t));

        if (!tok_is("}", self->t)) {
            self->i = before;
            // XXX(cleanup): lstptr[..], lstptr
            fail("missing closing brace");
        }

        // XXX(cleanup): lstptr[..], lstptr
        if (!(r = calloc(1, sizeof *r))) fail("OOM");
        r->ty = LST;
        r->as.lst.ptr = lstptr;
        r->as.lst.len = lstlen;
        r->update = _update_free_lst;
    }

    else if (tok_is_sym(self->t)) {
        if (!(r = calloc(1, sizeof *r))) fail("OOM");
        r->ty = SYM;
        r->as.sym = slice2sym((++self->t.ptr, self->t));
    }

    else if (tok_is("_", self->t)) {
        r = self->unnamed;
        if (!r) fail("no unnamed variable at this point");
    }

    else if (tok_is_var(self->t)) {
        r = scope_get(scope, slice2sym(self->t));
        if (!r) fail("unknown variable");
    }

    if (atomic) return r;

    sz before = self->i;
    if (!_lex(self)) return r;
    if (!tok_is(",", self->t)) {
        self->i = before;
        return r;
    }

    self->unnamed = r;
    return _parse_expr(self, scope, false);
} // _parse_expr

bool _parse_script(Pars* self, Scope* scope) {
    do {
        if (!_lex(self)) return true;
        // XXX(cleanup): unnamed
        if (!tok_is_var(self->t)) fail("expected variable name");
        Slice const name = self->t;

        // XXX(cleanup): unnamed
        if (!_lex(self) || !tok_is("=", self->t)) fail("expected equal");

        Obj* value = _parse_expr(self, scope, false);
        // XXX(cleanup): unnamed
        if (!value) fail("in script expression");

        if (!tok_is("_", name)) {
            Sym const key = slice2sym(name);
            if (!scope_put(scope, key, value)) fail("OOM");
        }
    } while (_lex(self) && tok_is(";", self->t));

    return true;
}

bool lang_process(char const* script, Scope* scope) {
    Pars p = {.s= script, .i= 0};

    if (_parse_script(&p, scope)) return true;

    _print_location(&p, "ERROR");
    return false;
}

void lang_show_tokens(char const* script) {
    Pars p = {.s= script, .i= 0};
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

    if (!*p.t.ptr) printf("end of script\n");
    else _print_location(&p, "ERROR");
}
