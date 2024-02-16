#include "bidoof/base.h"
#include "bidoof/tools/images.h"

// buf: has .ptr and .len
// cref: const* const

void chans(buf cref filepath, buf cref channels, buf cref outfile) {
    buf const pngbuf = file_read(filepath);
    png_data const png = png_data_parse(&pngbuf);
    buf const rgba = png_get_pixels(&png, NULL);

    buf res = {0};
    for (sz i = 0; i < rgba.len; i+= 4)
        for (sz j = 0; j < channels->len; j++) switch (channels->ptr[j]|32) {
            case 'r': *dyarr_push(&res) = rgba.ptr[i+0]; break;
            case 'g': *dyarr_push(&res) = rgba.ptr[i+1]; break;
            case 'b': *dyarr_push(&res) = rgba.ptr[i+2]; break;
            case 'a': *dyarr_push(&res) = rgba.ptr[i+3]; break;
            default: exitf("unexpected channel character: '%c' (0x%02X)", channels->ptr[j], channels->ptr[j]);
        }

    file_write(outfile, &res);
}

void chunk(buf cref filepath, buf cref chunk, buf cref outfile) {
    buf const pngbuf = file_read(filepath);
    png_data const png = png_data_parse(&pngbuf);

    if (!strcmp("-l", (char*)chunk->ptr)) {
        buf const res = png_data_chunknames(&png);
        file_write(outfile, &res);
    }

    else if (4 != chunk->len) exitf("chunk name should be 4 characters, got '%.*s'", (unsigned)chunk->len, chunk->ptr);

    else {
        buf const res = png_data_find(&png, png_chunk_type_str(chunk->ptr));
        file_write(outfile, &res);
    }
}

make_main("png tool to get channels or chunks",
    make_cmd(chans(infile, channels, outfile), "read png, extract channels",
        make_arg_buf(infile, "path to a RGBA png file")
        make_arg_buf(channels, "which channels to extract, eg. 'BRA' for blue, red, alpha")
        make_arg_buf(outfile, "result")
    )
    make_cmd(chunk(infile, chunkname, outfile), "read png, extract chunk",
        make_arg_buf(infile, "path to a png file")
        make_arg_buf(chunkname, "a 4-character chunk name, or -l for a list")
        make_arg_buf(outfile, "result")
    )
)
