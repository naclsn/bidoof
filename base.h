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
typedef struct Sym { char const* ptr; sz len;                         } Sym;

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

typedef struct Entry {
    Sym key;
    Obj* value;
} Entry;

int symcmp(Sym const l, Sym const r);

void obj_show(Obj const* self, int indent);
void obj_show_depnts(Obj const* self, int curdepth);

Obj* obj_call(Obj* self, u8 argc, Obj** argv);
// remove self from dep's depnts; false if it was not in
bool obj_remdep(Obj* self, Obj* dep);
// do not destroy an Obj which has other Obj depending on it
// (ie. `self->depnts` not NULL)
void obj_destroy(Obj* self);
bool obj_update(Obj* self);

#endif // __BIDOOF_BASE_H__
