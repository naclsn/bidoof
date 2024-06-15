#undef _buizel_declonly
#ifdef BUIZEL_DECLONLY
#define _buizel_declonly(...) ;
#else
#define _buizel_declonly(...) __VA_ARGS__
#endif

// uNN/sz typedefs, buf typedef and dyarr_ macros, ref/cref macros
#include "../base.h"

typedef struct buizel_reader {
    buf cref src;
    sz at;
    unsigned _has, _acc;
} buizel_reader;

typedef struct buizel_writer {
    buf ref res;
    unsigned _space;
} buizel_writer;

u64 buizel_read(buizel_reader ref self, unsigned const nbits, unsigned ref actual) _buizel_declonly({
    u64 r = self->_acc;
    *actual = self->_has;

    while (*actual < nbits) {
        if (self->src->len <= self->at) break;
        u8 const b = self->src->ptr[self->at++];

        unsigned const take = nbits-*actual < 8 ? nbits-*actual : 8;

        // "little", as in reads the lowest bit first
        r|= (b & ((1<<take) -1)) << *actual;
        // "big", as in reads the lowest bit first
        //r = r<<take | (b & ((1<<take) -1)); // xxx: bit reverse?

        self->_acc = b>>take;
        self->_has = 8-take;

        *actual+= take;
    }

    return r;
})

unsigned buizel_left(buizel_reader cref self) _buizel_declonly({
    return (self->src->len - self->at) * 8 + self->_has;
})

void buizel_align(buizel_reader ref self) _buizel_declonly({
    self->_acc = self->_has = 0;
})

bool buizel_write(buizel_writer ref self, unsigned const nbits, u64 value) _buizel_declonly({
    unsigned const finish = self->_space < nbits ? self->_space : nbits;
    if (finish) {
        // "little"
        self->res->ptr[self->res->len-1]|= (value & ((1<<finish) -1)) << (8-self->_space);
        value>>= finish;

        if (self->_space-= finish) return true;
    }

    unsigned const fullbytes = (nbits-finish)/8;

    while (self->res->cap <= self->res->len + fullbytes+1)
        if (!dyarr_resize(self->res, self->res->cap ? self->res->cap*2 : 16))
            return false;

    for (unsigned k = 0; k < fullbytes; k++) {
        // for "big": xxx: bit reverse?
        self->res->ptr[self->res->len++] = value&0xff;
        value>>= 8;
    }

    unsigned const left = nbits-finish-fullbytes*8;
    if (left) {
        // "little"
        self->res->ptr[self->res->len++] = value & ((1<<left) -1);
        self->_space = 8-left;
    }

    return true;
})

unsigned buizel_right(buizel_writer cref self) _buizel_declonly({
    return self->res->len * 8 - self->_space;
})

void buizel_pad(buizel_writer ref self) _buizel_declonly({
    self->_space = 0;
})
