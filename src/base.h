#ifndef __BIDOOF_BASE_H__
#define __BIDOOF_BASE_H__

#include <alloca.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef TRACE_ALLOCS
extern void* __trace_allocs_malloc(char const* const info, size_t const s);
extern void  __trace_allocs_free(char const* const info, void* const p);
extern void* __trace_allocs_calloc(char const* const info, size_t const c, size_t const s);
extern void* __trace_allocs_realloc(char const* const info, void* const p, size_t const s);
#define __trace_allocs_lnstr(__ln) #__ln
#define __trace_allocs_stamp(__fn, __ln) __fn ":" __trace_allocs_lnstr(__ln)
#define malloc(__s)        __trace_allocs_malloc (" // " __trace_allocs_stamp(__FILE__, __LINE__) ": malloc("  #__s           ")\n", (__s)       )
#define free(__p)          __trace_allocs_free   (" // " __trace_allocs_stamp(__FILE__, __LINE__) ": free("    #__p           ")\n", (__p)       )
#define calloc(__c, __s)   __trace_allocs_calloc (" // " __trace_allocs_stamp(__FILE__, __LINE__) ": calloc("  #__c ", " #__s ")\n", (__c), (__s))
#define realloc(__p, __s)  __trace_allocs_realloc(" // " __trace_allocs_stamp(__FILE__, __LINE__) ": realloc(" #__p ", " #__s ")\n", (__p), (__s))
#endif // TRACE_ALLOCS

#include "dyarr.h"

#define frommember(__it, __type, __member)  (  (__type*)( ((char*)__it) - offsetof(__type, __member) )  )

// usefull macros used in helper.h and bipa.h
#define _UNPACK(...) __VA_ARGS__
#define _CALL(__macro, ...) __macro(__VA_ARGS__)

#define _FOR_TYNM_1(__n, __macro, __ty, __nm)       __macro((__n-1), __n, __ty, __nm)
#define _FOR_TYNM_2(__n, __macro, __ty, __nm, ...)  __macro((__n-2), __n, __ty, __nm) _FOR_TYNM_1(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM_3(__n, __macro, __ty, __nm, ...)  __macro((__n-3), __n, __ty, __nm) _FOR_TYNM_2(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM_4(__n, __macro, __ty, __nm, ...)  __macro((__n-4), __n, __ty, __nm) _FOR_TYNM_3(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM_5(__n, __macro, __ty, __nm, ...)  __macro((__n-5), __n, __ty, __nm) _FOR_TYNM_4(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM_6(__n, __macro, __ty, __nm, ...)  __macro((__n-6), __n, __ty, __nm) _FOR_TYNM_5(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM_7(__n, __macro, __ty, __nm, ...)  __macro((__n-7), __n, __ty, __nm) _FOR_TYNM_6(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM_8(__n, __macro, __ty, __nm, ...)  __macro((__n-8), __n, __ty, __nm) _FOR_TYNM_7(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM_9(__n, __macro, __ty, __nm, ...)  __macro((__n-9), __n, __ty, __nm) _FOR_TYNM_8(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM_10(__n, __macro, __ty, __nm, ...)  __macro((__n-10), __n, __ty, __nm) _FOR_TYNM_9(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM_11(__n, __macro, __ty, __nm, ...)  __macro((__n-11), __n, __ty, __nm) _FOR_TYNM_10(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM_12(__n, __macro, __ty, __nm, ...)  __macro((__n-12), __n, __ty, __nm) _FOR_TYNM_11(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM_13(__n, __macro, __ty, __nm, ...)  __macro((__n-13), __n, __ty, __nm) _FOR_TYNM_12(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM_14(__n, __macro, __ty, __nm, ...)  __macro((__n-14), __n, __ty, __nm) _FOR_TYNM_13(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM_15(__n, __macro, __ty, __nm, ...)  __macro((__n-15), __n, __ty, __nm) _FOR_TYNM_14(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM_16(__n, __macro, __ty, __nm, ...)  __macro((__n-16), __n, __ty, __nm) _FOR_TYNM_15(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM_17(__n, __macro, __ty, __nm, ...)  __macro((__n-17), __n, __ty, __nm) _FOR_TYNM_16(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM_18(__n, __macro, __ty, __nm, ...)  __macro((__n-18), __n, __ty, __nm) _FOR_TYNM_17(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM_19(__n, __macro, __ty, __nm, ...)  __macro((__n-19), __n, __ty, __nm) _FOR_TYNM_18(__n, __macro, __VA_ARGS__)
#define _FOR_TYNM(__n, __macro, ...)  _FOR_TYNM_##__n(__n, __macro, __VA_ARGS__)

void notify_default(char const* s);
void notify_null(char const* s);
extern void (*notify)(char const* s);
// TODO: remove maybe
#define notify_printf(__sz, __fmt, ...) do {    \
        char* m = alloca(__sz);                 \
        snprintf(m, __sz, __fmt, __VA_ARGS__);  \
        notify(m);                              \
    } while (false)                             \

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;
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
    void* data;

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

#define _symcvt_cmp_1(__sym, __v) 0 == symcmp(mksym(#__v), __sym) ? (__v)
#define _symcvt_cmp_2(__sym, __v, ...) _symcvt_cmp_1(__sym, __v) : _symcvt_cmp_1(__sym, __VA_ARGS__)
#define _symcvt_cmp_3(__sym, __v, ...) _symcvt_cmp_1(__sym, __v) : _symcvt_cmp_2(__sym, __VA_ARGS__)
#define _symcvt_cmp_4(__sym, __v, ...) _symcvt_cmp_1(__sym, __v) : _symcvt_cmp_3(__sym, __VA_ARGS__)
#define _symcvt_cmp_5(__sym, __v, ...) _symcvt_cmp_1(__sym, __v) : _symcvt_cmp_4(__sym, __VA_ARGS__)
#define _symcvt_cmp_6(__sym, __v, ...) _symcvt_cmp_1(__sym, __v) : _symcvt_cmp_5(__sym, __VA_ARGS__)
#define _symcvt_cmp_7(__sym, __v, ...) _symcvt_cmp_1(__sym, __v) : _symcvt_cmp_6(__sym, __VA_ARGS__)
#define _symcvt_cmp_8(__sym, __v, ...) _symcvt_cmp_1(__sym, __v) : _symcvt_cmp_7(__sym, __VA_ARGS__)
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
