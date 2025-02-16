old version of a toolbox that I worked on in c

### the name was just supposed to be a placeholder for real but here we are and now `README.md` has been crossed out of the to-do list yes that is probably good enough for now

framework-ish thing to play around with files of bytes

requirements:
- a `cc` (with c99)
- a `make` (somewhat optional)
- (opengl if using `frame.h`, or through `views.h`)

```console
$ mkdir build/
$ make build/example
$ build/example
```

also provided for convenience: `./run example.c`
(with or without the .c, but your shell's completion will probably put it)

or without `make`: `cc example.c -DBIDOOF_IMPLEMENTATION && ./a.out`

---

see [example.c](example.c) as for how to structure a program

`make build/example` command will generate 2 executables:
- `build/d-example.exe`
- `build/example`

the `build/d-%.exe` lists the dependencies of the program
it is executed right away to build the `build/%`

---

`make_main` generates an entry point with the descriptions and such
and accepting the commands given in `make_cmd` (only the name of the function)

for the rest, it might be necessary to swim through sources and parse unreasonable macro-based "templates"
but basically the things are in [bidoof/tools/](bidoof/tools/)

[bidoof/utils/](bidoof/utils/) contains somewhat more internal and obscure stuff:
- `bipa.h`: builder+parser generator for (byte-wise) binary formats
- `dyarr.h`: the backing things for the `buf` type
- `frame.h`: minimal open a window with GL context (tested with Win32, X11)
- `paras.h`: (limited) parser+assembler generator
- `buizel.h`: (somewhat wip) binary stream read/write

---

the `.gitignore` lists a `unreleased/`
it is essentially a private unversioned space for testing and development
if there is a `unreleased/Makefile`, any tool it lists in `x-toolnames` will be integrated the same way as the normal `toolnames` (in `bidoof/tools/`)
see the comment in the main [Makefile](Makefile)

---

```NOTES.md
extends TextureRect
TODO: doc comments more or less everywhere

- deflates less dumb (api?)
- filters less dumb (api?)

aec (what this?)
idn (what this?)
```
