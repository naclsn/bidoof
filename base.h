#ifndef __BIDOOF_BASE_H__
#define __BIDOOF_BASE_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef size_t sz;

typedef struct Obj Obj;
typedef struct Buf Buf;
typedef struct Num Num;
typedef struct Lst Lst;
typedef struct Fun Fun;
typedef struct Sym Sym;

struct Obj {
    bool (*update)(Obj* self);

    enum { BUF, NUM, LST, FUN, SYM } ty;

    union {
        struct Buf { u8* ptr; sz len;                   } buf;
        struct Num { int val;                           } num;
        struct Lst { Obj* ptr; sz len;                  } lst;
        struct Fun { bool (*call)(Obj* self, Obj* res); } fun;
        struct Sym { char const* ptr; sz const len;     } sym;
    } as;

    struct Depnt {
        Obj* obj;
        struct Depnt* next;
    }* depnts;
    u16 cycle;

    u8 argc;
    Obj const* argv[];
};

void show2(Obj const* self, int indent);
Obj* show(Obj* self);
void showDepnts(Obj const* self, int depth);

Obj* call(Obj* self, u8 argc, Obj** argv);
void delete(Obj* self);
bool update(Obj* self);

#endif // __BIDOOF_BASE_H__
