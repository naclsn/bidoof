#ifndef __DYARR_H__
#define __DYARR_H__

#include <stdlib.h>
#include <stdbool.h>

#define dyarr(__elty)  \
    struct {  \
        __elty* ptr;  \
        size_t len, cap;  \
    }

static inline bool _dyarr_resize(void* ptr, size_t* cap, size_t rsz, size_t isz) {
    void** _ptr = ptr;
    if (rsz) {
        void* niw = realloc(*_ptr, rsz * isz);
        if (!niw) return false;
        *_ptr = niw;
    }
    *cap = rsz;
    return true;
}

// clears it empty and free used memory
#define dyarr_clear(__da)  ((__da)->len = (__da)->cap = 0, free((__da)->ptr), (__da)->ptr = NULL)

// resize exactly to given, except if requesting 0 (only the capacity is ajusted)
#define dyarr_resize(__da, __res)  _dyarr_resize(&(__da)->ptr, &(__da)->cap, __res, sizeof*(__da)->ptr)

// double the capacity if more memory is needed
#define dyarr_push(__da)  ((__da)->len < (__da)->cap || dyarr_resize(__da, (__da)->cap ? (__da)->cap * 2 : 16) ? &(__da)->ptr[(__da)->len++] : NULL)

// NULL if empty
#define dyarr_pop(__da)  ((__da)->len ? &(__da)->ptr[(__da)->len--] : NULL)

// NULL if out of range
#define dyarr_at(__da, __k)  (0 <= __k && __k < (__da)->len ? &(__da)->ptr[__k] : NULL)

#endif // __DYARR_H__
