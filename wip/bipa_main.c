#include <stdio.h>

#define BIPA_HIDUMP
#include "../src/bipa.h"

bipa_union(local_file_header_signature, 1, (void), (u32le, 0x04034b50, _))
bipa_struct(local_file_header, 13
        , (union, local_file_header_signature), _
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
        , (lstr, self->file_name_length), file_name
        , (lstr, self->extra_field_length), extra_field
        )

//bipa_struct(encryption_header, 1, (u8), _)

// "[..] with or without this signature marking data descriptors"
bipa_union(data_descriptor_maybe_signature, 2, (void), (u32le, 0x08074b50, yes), (void), (void, 0, no))
bipa_struct(data_descriptor, 4
        , (union, data_descriptor_maybe_signature), _
        , (u32le), crc_32
        , (u32le), compressed_size
        , (u32le), uncompressed_size
        )

bipa_struct(local_file, 2
        , (struct, local_file_header), local_file_header
        //, (struct, encryption_header), encryption_header // only if encrypted
        , (lstr, self->local_file_header.compressed_size), file_data
        //, (struct, data_descriptor), data_descriptor // only if some random bit
        )
bipa_array(local_files, (struct, local_file))

bipa_union(central_directory_header_signature, 1, (void), (u32le, 0x02014b50, _))
bipa_struct(central_directory_header, 20
        , (union, central_directory_header_signature), _
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
        , (lstr, self->file_name_length), file_name
        , (lstr, self->extra_field_length), extra_field
        , (lstr, self->file_comment_length), file_comment
        )
bipa_array(central_directory_headers, (struct, central_directory_header))

bipa_struct(extra_field_header, 2
        , (u16le), id
        , (u16le), size
        )
bipa_struct(extra_field, 2
        , (struct, extra_field_header), header
        , (lstr, self->header.size), data
        )
bipa_array(extra_fields, (struct, extra_field));
bipa_struct(extra_fields_wrap, 1 , (array, extra_fields, pa->at < pa->buf->len), w);

bipa_union(digital_signature_header_signature, 1, (void), (u32le, 0x05054b50, _))
bipa_struct(digital_signature, 3
        , (union, digital_signature_header_signature), _
        , (u16le), size_of_data
        , (lstr, self->size_of_data), signature_data
        )

bipa_struct(zip_file, 2
        , (array, local_files, !memcmp(pa->buf->ptr+pa->at, "\x50\x4b\x03\x04", 4)), local_files
        , (array, central_directory_headers, !memcmp(pa->buf->ptr+pa->at, "\x50\x4b\x01\x02", 4)), central_directory_headers
        )

void xxd(Buf const* const b) {
    for (sz k = 0; k < b->len; k++) {
        if (k && !(k & 0xf)) printf("\n");
        printf("%02X ", b->ptr[k]);
    }
}

int main(int argc, char** argv) {
    char const* fn = argv[1];
    if (argc < 2 || !fn || !*fn) fn = "aa.zip"; //return puts("missing file name");

    Buf b;
    {
        FILE* f = fopen(fn, "rb");
        if (!f) return printf("could not open file %s\n", fn);

        fseek(f, 0, SEEK_END);
        b.len = ftell(f);
        b.ptr = malloc(b.len);
        if (!b.ptr) return puts("lol oom? really?");
        fseek(f, 0, SEEK_SET);
        printf("(read %zub)\n", fread(b.ptr, 1, b.len, f));
        fclose(f);
    }

    //xxd(&b);

    BufParser p = {.buf= &b};
    struct zip_file res;
    bipa_parse_zip_file(&res, &p);

    puts("res:");
    bipa_dump_zip_file(&res, 0);
    puts("\n---\n");

    for (size_t k = 0; k < res.central_directory_headers.len; k++) {
        struct central_directory_header const* it = res.central_directory_headers.ptr+k;
        BufParser ex_p = {.buf= &(Buf const){.len= it->extra_field_length, .ptr= it->extra_field}};
        struct extra_fields_wrap fields;
        bipa_parse_extra_fields_wrap(&fields, &ex_p);
        printf("%.*s extra fields:\n", (int)it->file_name_length, it->file_name);
        bipa_dump_extra_fields_wrap(&fields, 0);
        puts("");
        bipa_free_extra_fields_wrap(&fields);
    }

    bipa_free_zip_file(&res);

    free(b.ptr);
    return 0;
}
