### the name was just supposed to be a placeholder for real but here we are and now `README.md` has been crossed out of the to-do list yes that is probably good enough for now

```console
$ make  # result in build/
$ build/bidoof
```

To count memory leaks (there's a lot):
```console
$ make CFLAGS=-DTRACE_ALLOCS
$ build/bidoof 2> >(awk -f etc/trace_allocs_check_tape.awk)
```

`ext/` contains a set of dynamic libraries that are manually loaded.
These each contain `Meta`s (wrappers around object, mostly functions) around a thing:
- `builtin`: set of basic operations on objects
- `encodings`: encoding/decoding functions (eg. base 64, ...)
- `views`: visualisation, (eg. buffer as text, as image, ...)

> Note: `ext/views` builds on Windows or with X11.

Objects can be:
- `Buf`: buffer of bytes (assumed octets)
- `Num`: (for now) 64 bits signed integer
- `Flt`: (for now) 64 bits floating point
- `Lst`: list of objects (doesn't have to be homogeneous)
- `Fun`: function (can have multiple overloads)
- `Sym`: symbols (at most 15 characters)

Most janky DSL ever to play with these (Vim filetype in `etc/vim/`):
```
<script> ::= <var> '=' <expr> {';' <var> '=' <expr>} [';']
<expr> ::
     = <str> | <num> | <lst> | <fun> | <sym> | <var>
     | <fun> <expr> {<expr>}
     | '(' <expr> ')'
     | <expr> ',' <expr>
```
Comments starts with a `#`. Numbers are the usual (.. `0b`, `0o`, `0x`). Character literals with `'` are also numbers. String literals with `"` are buffers. Lists are written with `{}`. Symbols start with a `:`, like `:name`. Function names are `CamelCase` and variable names are `snake_case`. Finally `,` is like the shell pipe `|`: `_` after a `,` is the result of the expression before.
