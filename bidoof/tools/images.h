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

bipa_union(color_type, 5
        , (void,), (u8, 0, grayscale)
        , (void,), (u8, 2, truecolor)
        , (void,), (u8, 3, indexed_color)
        , (void,), (u8, 4, grauscale_with_alpha)
        , (void,), (u8, 6, truecolor_with_alpha)
        )
bipa_union(compression_method, 1, (void,), (u8, 0, deflate))
bipa_union(filter_method, 1, (void,), (u8, 0, method_0))
bipa_union(interlace_method, 2
        , (void,), (u8, 0, no_interlacing)
        , (void,), (u8, 1, adam_7)
        )
bipa_struct(png_chunk_header, 7
        , (u32be,), width
        , (u32be,), height
        , (u8,), bit_depth
        , (union, color_type), color_type
        , (union, compression_method), compression_method
        , (union, filter_method), filter_method
        , (union, interlace_method), interlace_method
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
