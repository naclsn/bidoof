#include <stdio.h>

#define BIPA_HIDUMP
#include "../src/bipa.h"

bipa_struct(local_file_header, 10
    // header signature: 0x04034b50
    , (u16le), version_needed_to_extract
    , (u16le), general_purpose_bit_flag
    , (u16le), compression_method
    , (u16le), last_mod_file_time
    , (u16le), last_mod_file_date
    , (u32le), crc_32
    , (u32le), compressed_size
    , (u32le), uncompressed_size
    , (u16le), file_name_length
    , (u16le), extra_field_length
    //, (lstr, self->file_name_length), file_name
    //, (lstr, self->extra_field_length), extra_field
)
//bipa_struct(encryption_header, 1, (u8), _)
bipa_struct(data_descriptor, 3
    // header signature: 0x08074b50 - "[..] with or without this signature marking data descriptors"
    , (u32le), crc_32
    , (u32le), compressed_size
    , (u32le), uncompressed_size
)
bipa_struct(local_file, 2
    , (struct, local_file_header), local_file_header
    //, (struct, encryption_header), encryption_header // only if encrypted
    //, (lstr, self->local_file_header.compressed_size), file_data
    , (struct, data_descriptor), data_descriptor // "when it was not possible to seek in the output"
)

bipa_array(local_files, (struct, local_file))

bipa_struct(central_directory_header, 16
    // header signature: 0x02014b50
    , (u16le), version_made_by
    , (u16le), version_needed_to_extract
    , (u16le), general_purpose_bit_flag
    , (u16le), compression_method
    , (u16le), last_mod_file_time
    , (u16le), last_mod_file_date
    , (u32le), crc_32
    , (u32le), compressed_size
    , (u32le), uncompressed_size
    , (u16le), file_name_length
    , (u16le), extra_field_length
    , (u16le), file_comment_length
    , (u16le), disk_number_start
    , (u16le), internal_file_attributes
    , (u32le), external_file_attributes
    , (u32le), relative_offset_of_local_header
    //, (lstr, self->file_name_length), file_name
    //, (lstr, self->extra_field_length), extra_field
    //, (lstr, self->file_comment_length), file_comment
)
bipa_array(central_directory_headers, (struct, central_directory_header))
bipa_struct(digital_signature, 1
    // header signature: 0x05054b50
    , (u16le), size_of_data
    //, (lstr, self->size_of_data), signature_data
)

void xxd(Buf const* const b) {
    for (sz k = 0; k < b->len; k++) {
        if (k && !(k & 0xf)) printf("\n");
        printf("%02X ", b->ptr[k]);
    }
}

bipa_struct(test_cstr, 1
        , (cstr, '\0'), data
        )

int main_cstr(void) {
    struct test_cstr src = {.data= (u8*)"hi :3"};
    struct test_cstr res;

    puts("src:"); bipa_dump_test_cstr(&src, 0); puts("");

    BufBuilder builder = {0};
    bipa_build_test_cstr(&src, &builder);
    Buf b = {.len= builder.arr.len, .ptr= builder.arr.ptr};
    puts("buf:"); xxd(&b); puts("");
    BufParser parser = {.buf= &b};
    bipa_parse_test_cstr(&res, &parser);
    free(b.ptr);

    puts("res:"); bipa_dump_test_cstr(&res, 0); puts("");

    return 0;
}

bipa_struct(test_lstr, 2
        , (u16be), length
        , (lstr, self->length), data
        )

int main_lstr(void) {
    struct test_lstr src = {.length= 5, .data= (u8*)"hi :3"};
    struct test_lstr res;

    puts("src:"); bipa_dump_test_lstr(&src, 0); puts("");

    BufBuilder builder = {0};
    bipa_build_test_lstr(&src, &builder);
    Buf b = {.len= builder.arr.len, .ptr= builder.arr.ptr};
    puts("buf:"); xxd(&b); puts("");
    BufParser parser = {.buf= &b};
    bipa_parse_test_lstr(&res, &parser);
    free(b.ptr);

    puts("res:"); bipa_dump_test_lstr(&res, 0); puts("");

    return 0;
}

bipa_union(test_union, 4
        , (void), (u8, 42, nothing)
        , (u16le), (u8, 48, ushort)
        , (u32le), (u8, 49, uint)
        , (u64le), (u8, 50, ulong)
        )

int main_union(void) {
    struct test_union src = {
        .val.uint= 42,
        //.tag= test_union_tag_uint,
        .tag= test_union_tag_nothing,
    };
    struct test_union res;

    puts("src:"); bipa_dump_test_union(&src, 0); puts("");

    BufBuilder builder = {0};
    bipa_build_test_union(&src, &builder);
    Buf b = {.len= builder.arr.len, .ptr= builder.arr.ptr};
    puts("buf:"); xxd(&b); puts("");
    BufParser parser = {.buf= &b};
    bipa_parse_test_union(&res, &parser);
    free(b.ptr);

    puts("res:"); bipa_dump_test_union(&res, 0); puts("");

    return 0;
}

bipa_union(maybe, 2
        , (u64le), (u32le, 0x08074b50, yes)
        , (u64le), (void, 0, no)
        )

int main_maybe(void) {
    struct maybe src = {
        .val= {9876543210},
        //.tag= maybe_tag_yes,
        .tag= maybe_tag_no,
    };
    struct maybe res;

    puts("src:"); bipa_dump_maybe(&src, 0); puts("");

    BufBuilder builder = {0};
    bipa_build_maybe(&src, &builder);
    Buf b = {.len= builder.arr.len, .ptr= builder.arr.ptr};
    puts("buf:"); xxd(&b); puts("");
    BufParser parser = {.buf= &b};
    bipa_parse_maybe(&res, &parser);
    free(b.ptr);

    puts("res:"); bipa_dump_maybe(&res, 0); puts("");

    return 0;
}

int main(void) {
    return 0
    //+ main_cstr()
    //+ main_lstr()
    //+ main_union()
    + main_maybe()
    ;
}
