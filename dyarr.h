#ifndef __DYARR_H__
#define __DYARR_H__

#include <stdlib.h>
#include <stdbool.h>

#define dyarr(__elty)  \
    struct {  \
        __elty* ptr;  \
        size_t len, cap;  \
    }

static inline bool _dyarr_resize(void** ptr, size_t* cap, void* nptr, size_t ncap) {
    if (!nptr) return false;
    *ptr = nptr;
    *cap = ncap;
    return true;
}

// clears it empty and free used memory
#define dyarr_clear(__da)  ((__da)->len = (__da)->cap = 0, free((__da)->ptr), (__da)->ptr = NULL)

// resize exactly to given, new size should not be 0
// (FIXME: __res is evaluated twice, but I really want the realloc to be in the macro, not in the function)
#define dyarr_resize(__da, __res)  _dyarr_resize((void*)&(__da)->ptr, &(__da)->cap, realloc((__da)->ptr, (__res)*sizeof*(__da)->ptr), (__res))

// double the capacity if more memory is needed
#define dyarr_push(__da)  ((__da)->len < (__da)->cap || dyarr_resize(__da, (__da)->cap ? (__da)->cap * 2 : 16) ? &(__da)->ptr[(__da)->len++] : NULL)

// NULL if empty
#define dyarr_pop(__da)  ((__da)->len ? &(__da)->ptr[(__da)->len--] : NULL)

// NULL if out of range
#define dyarr_at(__da, __k)  (0 <= __k && __k < (__da)->len ? &(__da)->ptr[__k] : NULL)

// NULL if OOM, else pointer to the new sub-array (at k, of size n)
#define dyarr_insert(__da, __k, __n)  (__n+(__da)->len < (__da)->cap || dyarr_resize(__da, __n+(__da)->len) ? memmove( __k+(__n)+(__da)->ptr, __k+(__da)->ptr, __k+(__n)-(__da)->len), (__da)->len+= __n, __k+(__da)->ptr : NULL)

// (you have to declare your iterator, and you don't get enumerator either) `some* it; dyarr_for(&some_dyarr, it) { .. }`
#define dyarr_for(__da, __it)  for (__it = (__da)->ptr; (size_t)(__it-(__da)->ptr) < (__da)->len; ++__it)

#endif // __DYARR_H__
