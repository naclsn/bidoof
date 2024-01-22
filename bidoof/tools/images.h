#ifndef BDF_IMPLEMENTATION
#define BIPA_DECLONLY
#endif
#define BIPA_HIDUMP
#include "../bipa.h"

#define _png_chunk_id(a,b,c,d) ((a<<24)|(b<<16)|(c<<8)|d)

bipa_struct(png_chunk_type_other, 1, (lstr, 4), name)
bipa_union(png_chunk_type, 3
        , (void,), (u32be, _png_chunk_id('I','H','D','R'), IHDR)
        , (void,), (u32be, _png_chunk_id('I','E','N','D'), IEND)
        , (struct, png_chunk_type_other), (void, 0, other)
        )

bipa_struct(png_chunk_header, 7
        , (u32be,), width
        , (u32be,), height
        , (u8,), bit_depth
        , (u8,), color_type
        , (u8,), compression_method
        , (u8,), filter_method
        , (u8,), interlace_method
        )

bipa_struct(png_chunk, 4
        , (u32be,), length
        , (union, png_chunk_type), type
        , (lstr, self->length), data
        , (u32be,), crc
        )
bipa_array(png_chunks, (struct, png_chunk))

bipa_union(png_magic_number, 1, (void,), (u64be, 0x89504e470d0a1a0aLL, _))
bipa_struct(png_data, 2
        , (union, png_magic_number), _
        , (array, png_chunks, !k || png_chunk_type_tag_IEND != it->ptr[k-1].type.tag), chunks
        )

// https://www.w3.org/TR/2003/REC-PNG-20031110/
adapt_bipa_type(png_data)
buf png_data_get(png_data cref png, int const tag);
adapt_bipa_type(png_chunk_header)

#ifdef BDF_IMPLEMENTATION

buf png_data_get(png_data cref png, int const tag) {
    for (sz k = 0; k < png->chunks.len; k++)
        if (tag == (int)png->chunks.ptr[k].type.tag)
            return bufcpy(png->chunks.ptr[k].data, png->chunks.ptr[k].length);
    exitf("no chunk in png_data for given tag %d", tag);
    return (buf){0};
}

#endif // BDF_IMPLEMENTATION
