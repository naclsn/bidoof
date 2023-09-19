#ifndef __BIDOOF_BASE_H__
#define __BIDOOF_BASE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define frommember(__it, __type, __member)  (  (__type*)( ((char*)__it) - offsetof(__type, __member) )  )

void notify_default(char const* s);
void notify_null(char const* s);
extern void (*notify)(char const* s);
#define notify_printf(__sz, __fmt, ...) do {  \
        char* m = malloc(__sz);               \
        if (!m) notify("OOM");                \
        else {                                \
            sprintf(m, __fmt, __VA_ARGS__);   \
            notify(m);                        \
            free(m);                          \
        }                                     \
    } while (false)                           \

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int64_t i64;
typedef long double f64;
typedef size_t sz;

typedef struct Buf { u8* ptr; sz len;                                 } Buf;
typedef struct Num { i64 val;                                         } Num;
typedef struct Flt { f64 val;                                         } Flt;
typedef struct Lst { struct Obj** ptr; sz len;                        } Lst;
typedef struct Fun { bool (*call)(struct Obj* self, struct Obj* res); } Fun;
typedef struct Sym { char txt[16];                                    } Sym;

enum  Ty { BUF,     NUM,     FLT,     LST,     FUN,     SYM,     ANY };
union As { Buf buf; Num num; Flt flt; Lst lst; Fun fun; Sym sym; };

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

static inline int symcmp(Sym const l, Sym const r) {
    return memcmp(l.txt, r.txt, 16);
}

static inline Sym mksym(char const* s) {
    Sym r = {0};
    strncpy(r.txt, s, 15);
    return r;
}

#define _symcvt_cmp_1(__sym, __l, __v) 0 == symcmp(__sym, {.ptr= #__l, .len= strlen(#__l)) ? (__v)
#define _symcvt_cmp_2(__sym, __l, __v, ...) _symcvt_cmp_1(__sym, __l, __v) : _symcvt_cmp_1(__sym, __VA_ARGS__)
#define _symcvt_cmp_3(__sym, __l, __v, ...) _symcvt_cmp_1(__sym, __l, __v) : _symcvt_cmp_2(__sym, __VA_ARGS__)
#define _symcvt_cmp_4(__sym, __l, __v, ...) _symcvt_cmp_1(__sym, __l, __v) : _symcvt_cmp_3(__sym, __VA_ARGS__)
#define _symcvt_cmp_5(__sym, __l, __v, ...) _symcvt_cmp_1(__sym, __l, __v) : _symcvt_cmp_4(__sym, __VA_ARGS__)
#define _symcvt_cmp_6(__sym, __l, __v, ...) _symcvt_cmp_1(__sym, __l, __v) : _symcvt_cmp_5(__sym, __VA_ARGS__)
#define _symcvt_cmp_7(__sym, __l, __v, ...) _symcvt_cmp_1(__sym, __l, __v) : _symcvt_cmp_6(__sym, __VA_ARGS__)
#define _symcvt_cmp_8(__sym, __l, __v, ...) _symcvt_cmp_1(__sym, __l, __v) : _symcvt_cmp_7(__sym, __VA_ARGS__)
#define symcvt(__n, __sym, ...) (_symcvt_cmp_##__n(__sym, __VA_ARGS__) : -1)

void obj_show(Obj const* self, int indent);
void obj_show_depnts(Obj const* self, int curdepth);

// self is a function, the argument, and later result, are all in res
bool obj_call(Obj* self, Obj* res);
// remove self from dep's depnts and decrement its keepalive
// false if it was not in
bool obj_remdep(Obj* self, Obj* dep);
// do not destroy an Obj which has other Obj depending on it
// (ie. `self->keepalive` non-zero)
// destroying is recursive and assumes the arguments are allocated
// (these are freed if identified as not used)
void obj_destroy(Obj* self);
bool obj_update(Obj* self);

#endif // __BIDOOF_BASE_H__
