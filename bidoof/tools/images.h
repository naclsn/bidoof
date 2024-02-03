#ifndef __BIDOOF_T_IMAGES__
#define __BIDOOF_T_IMAGES__

#ifdef BIDOOF_T_IMPLEMENTATION
#define _redef_after_image
#undef BIDOOF_IMPLEMENTATION
#undef BIDOOF_T_IMPLEMENTATION
#endif
#include "compressions.h"
#ifdef _redef_after_image
#undef _redef_after_image
#define BIDOOF_IMPLEMENTATION
#define BIDOOF_T_IMPLEMENTATION
#endif

#include "../base.h"
#ifndef BIDOOF_IMPLEMENTATION
#define BIPA_DECLONLY
#endif
#define BIPA_HIDUMP
#include "../utils/bipa.h"

#ifdef BIDOOF_LIST_DEPS
static struct _list_deps_item const _list_deps_me_images = {_list_deps_first, "images"};
#undef _list_deps_first
#define _list_deps_first &_list_deps_me_images
#endif

#define png_chunk_type(a, b, c, d) (u32)((u32)(a<<24) | (u32)(b<<16) | (u32)(c<< 8) | (u32)(d<< 0))
#define png_chunk_type_str(s) png_chunk_type(s[0], s[1], s[2], s[3])

bipa_union(color_type, 5
        , (void,), (u8, 0, grayscale)
        , (void,), (u8, 2, truecolor)
        , (void,), (u8, 3, indexed_color)
        , (void,), (u8, 4, grayscale_with_alpha)
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
        , (u32be,), type
        , (lstr, self->length), data
        , (u32be,), crc
        )
bipa_array(png_chunks, (struct, png_chunk))

bipa_union(png_magic_number, 1, (void,), (u64be, 0x89504e470d0a1a0aLL, _))
bipa_struct(png_data, 2
        , (union, png_magic_number), _
        , (array, png_chunks, !k || png_chunk_type_str("IEND") != it->ptr[k-1].type), chunks
        )

// https://www.w3.org/TR/2003/REC-PNG-20031110/
adapt_bipa_type(png_data)
adapt_bipa_type(png_chunk_header)

buf png_data_chunknames(png_data cref png);
buf png_data_find(png_data cref png, u32 const type);
buf png_data_get_all_data(png_data cref png);
buf png_unfilter(png_chunk_header cref png_h, buf cref source);
buf png_get_pixels(png_data cref png, png_chunk_header opcref header);

#ifdef BIDOOF_IMPLEMENTATION

buf png_data_chunknames(png_data cref png) {
    buf r = {0};
    if (!dyarr_resize(&r, png->chunks.len*5)) exitf("OOM");
    for (sz k = 0; k < png->chunks.len; k++) {
        u32 const type = png->chunks.ptr[k].type;
        r.ptr[r.len++] = type>>24 & 0xff;
        r.ptr[r.len++] = type>>16 & 0xff;
        r.ptr[r.len++] = type>> 8 & 0xff;
        r.ptr[r.len++] = type>> 0 & 0xff;
        r.ptr[r.len++] = '\0';
    }
    return r;
}

buf png_data_find(png_data cref png, u32 const type) {
    for (sz k = 0; k < png->chunks.len; k++)
        if (type == png->chunks.ptr[k].type)
            return bufcpy(png->chunks.ptr[k].data, png->chunks.ptr[k].length);
    exitf("no chunk in png_data for given type %c%c%c%c", type<<24 & 0xff, type<<16 & 0xff, type<< 8 & 0xff, type<< 0 & 0xff);
    return (buf){0};
}

buf png_data_get_all_data(png_data cref png) {
    buf r = {0};
    for (sz k = 0; k < png->chunks.len; k++)
        if (png_chunk_type_str("IDAT") == png->chunks.ptr[k].type)
            bufcat(&r, &(buf const){
                .ptr= png->chunks.ptr[k].data,
                .len= png->chunks.ptr[k].length
            });
    return r;
}

// TODO: interlacing

buf png_unfilter(png_chunk_header cref png_h, buf cref source) {
    buf r = {0};

    sz const chan = 4;
    if (color_type_tag_truecolor_with_alpha != png_h->color_type.tag || 8 != png_h->bit_depth)
        exitf("NIY: only 8 bits truecolor with alpha is supported");
    if (!dyarr_resize(&r, png_h->width * png_h->height * chan)) exitf("OOM");

#   define _x (r.ptr[r.len+j*chan+k])
#   define _a (r.ptr[r.len+(j-1)*chan+k])
#   define _b (r.ptr[r.len+(j-w)*chan+k])
#   define _c (r.ptr[r.len+(j-w-1)*chan+k])
#   define _f (source->ptr[at+j*chan+k])
#   define for_chan(...) do for (sz k = 0; k < chan; k++) { __VA_ARGS__; } while (0)

    sz const w = png_h->width, h = png_h->height;
    sz at = 0;
    sz const j = 0; // (for the macro)
    for (sz i = 0; i < h; i++, at+= w*chan, r.len+= w*chan) switch (source->ptr[at++]) {
        case 0: // none
            memcpy(r.ptr+r.len, source->ptr+at, w*chan);
            break;
        case 1: // sub
            for_chan(_x = _f);
            for (sz j = 1; j < w; j++) for_chan(_x = _f + _a);
            break;
        case 2: // up
            if (!i) memcpy(r.ptr+r.len, source->ptr+at, w*chan);
            else for (sz j = 0; j < w; j++) for_chan(_x = _f + _b);
            break;
        case 3: // average
            if (!i) {
                for_chan(_x = _f);
                for (sz j = 1; j < w; j++) for_chan(_x = _f + _a/2);
            } else {
                for_chan(_x = _f + _b/2);
                for (sz j = 1; j < w; j++) for_chan(_x = _f + (_a+_b)/2);
            }
            break;
        case 4: // Paeth
            for (sz j = 0; j < w; j++) for_chan(
                u8 const a = !j ? 0 : _a
                       , b = !i ? 0 : _b
                       , c = !i || !j ? 0 : _c;
                u8 const p = a+b-c;
                u8 const pa = p < a ? a-p : p-a
                       , pb = p < b ? b-p : p-b
                       , pc = p < c ? c-p : p-c;
                u8 const pr = pa <= pb && pa <= pc ? a : pb < pc ? b : c;
                _x = _f + pr;
            );
            break;
        default:
            free(r.ptr);
            exitf("unknown filter type: %d\n", source->ptr[at-1]);
    }

#   undef for_chan
#   undef _f
#   undef _c
#   undef _b
#   undef _a
#   undef _x

    return r;
}

buf png_get_pixels(png_data cref png, png_chunk_header opcref header) {
    buf idat = png_data_get_all_data(png);
    buf infl = inflate(&idat, NULL);

    buf r;
    if (!header) {
        buf ihdr = png_data_find(png, png_chunk_type_str("IHDR"));
        png_chunk_header info = png_chunk_header_parse(&ihdr);
        r = png_unfilter(&info, &infl);
    } else r = png_unfilter(header, &infl);

    return r;
}

#endif // BIDOOF_IMPLEMENTATION

#endif // __BIDOOF_T_IMAGES__
