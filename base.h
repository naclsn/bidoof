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

typedef struct Buf { u8* ptr; sz len;                                 } Buf;
typedef struct Num { int val;                                         } Num;
typedef struct Lst { struct Obj* ptr; sz len;                         } Lst;
typedef struct Fun { bool (*call)(struct Obj* self, struct Obj* res); } Fun;
typedef struct Sym { char const* ptr; sz const len;                   } Sym;

typedef struct Obj {
    bool (*update)(struct Obj* self);

    enum  { BUF,     NUM,     LST,     FUN,     SYM,     } ty;
    union { Buf buf; Num num; Lst lst; Fun fun; Sym sym; } as;

    struct Depnt {
        struct Obj* obj;
        struct Depnt* next;
    }* depnts;
    u16 cycle;

    u8 argc;
    struct Obj* argv[];
} Obj;

void show2(Obj const* self, int indent);
Obj* show(Obj* self);
void showDepnts(Obj const* self, int depth);

Obj* call(Obj* self, u8 argc, Obj** argv);
void destroy(Obj* self);
bool update(Obj* self);

#endif // __BIDOOF_BASE_H__
