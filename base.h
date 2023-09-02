#ifndef __BIDOOF_BASE_H__
#define __BIDOOF_BASE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define frommember(__it, __type, __member)  (  (__type*)( ((char*)__it) - offsetof(__type, __member) )  )

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef size_t sz;

typedef struct Buf { u8* ptr; sz len;                                 } Buf;
typedef struct Num { int val;                                         } Num;
typedef struct Lst { struct Obj** ptr; sz len;                        } Lst;
typedef struct Fun { bool (*call)(struct Obj* self, struct Obj* res); } Fun;
typedef struct Sym { char const* ptr; sz len;                         } Sym;

enum  Ty { BUF,     NUM,     LST,     FUN,     SYM,     };
union As { Buf buf; Num num; Lst lst; Fun fun; Sym sym; };

typedef struct Obj {
    bool (*update)(struct Obj* self);

    enum  Ty ty;
    union As as;

    struct Depnt {
        struct Obj* obj;
        struct Depnt* next;
    }* depnts;
    u16 cycle;
    u16 keepalive;

    u8 argc;
    struct Obj* argv[];
} Obj;

typedef struct Meta {
    char const* const doc;
    char const* const name;

    struct MetaOvl {
        enum Ty const ret;
        struct MetaOvlPrm {
            enum Ty const ty;
            char const* const name;
        } const* const params;
    } const* const overloads;

    Obj obj;
} Meta;

int symcmp(Sym const l, Sym const r);

#define _symcvt_cmp_1(__sym, __l, __v) 0 == symcmp(__sym, {.ptr= #__l, .len= strlen(#__l)) ? (__v)
#define _symcvt_cmp_2(__sym, __l, __v, ...) _symcvt_cmp_1(__sym, __l, __v) : _symcvt_cmp_1(__sym, __VA_ARGS__)
#define _symcvt_cmp_3(__sym, __l, __v, ...) _symcvt_cmp_1(__sym, __l, __v) : _symcvt_cmp_2(__sym, __VA_ARGS__)
#define _symcvt_cmp_4(__sym, __l, __v, ...) _symcvt_cmp_1(__sym, __l, __v) : _symcvt_cmp_3(__sym, __VA_ARGS__)
#define _symcvt_cmp_5(__sym, __l, __v, ...) _symcvt_cmp_1(__sym, __l, __v) : _symcvt_cmp_4(__sym, __VA_ARGS__)
#define symcvt(__n, __sym, ...) (_symcvt_cmp_##__n(__sym, __VA_ARGS__) : -1)

void obj_show(Obj const* self, int indent);
void obj_show_depnts(Obj const* self, int curdepth);

Obj* obj_call(Obj* self, u8 argc, Obj** argv);
// remove self from dep's depnts and decrement its keepalive
// false if it was not in
bool obj_remdep(Obj* self, Obj* dep);
// do not destroy an Obj which has other Obj depending on it
// (ie. `self->keepalive` non-zero)
// destroying is recursive, and frees `self`
void obj_destroy(Obj* self);
bool obj_update(Obj* self);

#endif // __BIDOOF_BASE_H__
