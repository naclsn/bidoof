#ifndef BDF_IMPLEMENTATION
#define BIPA_DECLONLY
#endif
#define BIPA_HIDUMP
#include "../bipa.h"

bipa_struct(png_data, 4
        , (u8,), r
        , (u8,), g
        , (u8,), b
        , (u8,), a
        )
adapt_bipa_type(png_data)

#ifdef BDF_IMPLEMENTATION



#endif // BDF_IMPLEMENTATION
