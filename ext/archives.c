/// https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT

#include "../src/helper.h"
#include "../src/bipa.h"

export_names
    ( "ZipBuild"
    , "ZipParse"
    );

/*
bipa_struct(zip_file,
    //bipa_union(plho,
    bipa_listof(...,
        bp_lit(0x04034b50), bipa_struct(local_file,
            bipa_struct(local_file_header,
                bipa_le_u2(version_needed_to_extract),
                bipa_le_u2(general_purpose_bit_flag),
                bipa_le_u2(compression_method),
                bipa_le_u2(last_mod_file_time),
                bipa_le_u2(last_mod_file_date),
                bipa_le_u4(crc_32),
                bipa_le_u4(compressed_size),
                bipa_le_u4(uncompressed_size),
                bipa_le_u2(file_name_length),
                bipa_le_u2(extra_field_length),
                bipa_cstr(file_name, file_name_length),
                bipa_cstr(extra_field, extra_field_length),
            ),
            bipa_struct(encryption_header),
            //bipa_struct(file_data),
            bipa_cstr(file_data, ),
            bipa_struct(data_descriptor),
        )
    )
);
*/

typedef struct ZipData {
    int _;
} ZipData;

ctor_simple(1, ZipBuild
        , "builds a zip archive buffer"
        , (2, BUF, _ZipBuild, LST, names, LST, buffers)
        );

bool _ZipBuild(Buf* self, Lst const* const names, Lst const* const buffers) {
    if (destroyed(self)) {
        free(self->ptr);
        return true;
    }
    (void)names;
    (void)buffers;
    fail("NIY: ZipBuild");
}

ctor_simple(1, ZipParse
        , "parses a buffer as a zip archive"
        , (1, SYM, _ZipParse, BUF, source)
        );

bool _ZipParse(Sym* self, Buf const* const source) {
    ZipData* data = getdata(self);
    if (destroyed(self)) {
        free(data);
        return true;
    }
    if (!data) {
        strcpy(self->txt, "#ZipParse");
        data = getdata(self) = malloc(sizeof(ZipData));
        if (!data) fail("OOM");
    }

    BufParser _p = {0}, * p = &_p;
    (void)source;
    //bp_from_buf(p, source);

    (void)p;

    return true; //fail("NIY: ZipParse");
}
