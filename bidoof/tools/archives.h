#ifndef BDF_IMPLEMENTATION
#define BIPA_DECLONLY
#endif
#define BIPA_HIDUMP
#include "../bipa.h"

bipa_union(local_file_header_signature, 1, (void,), (u32le, 0x04034b50, _))
bipa_struct(local_file_header, 13
        , (union, local_file_header_signature), _
        , (u16le,), version_needed_to_extract
        , (u16le,), general_purpose_bit_flag
        , (u16le,), compression_method
        , (u16le,), last_mod_file_time
        , (u16le,), last_mod_file_date
        , (u32le,), crc_32
        , (u32le,), compressed_size
        , (u32le,), uncompressed_size
        , (u16le,), file_name_length
        , (u16le,), extra_field_length
        , (lstr, self->file_name_length), file_name
        , (lstr, self->extra_field_length), extra_field
        )

//bipa_struct(encryption_header, 1, (u8), _)

// "[..] with or without this signature marking data descriptors"
bipa_union(data_descriptor_maybe_signature, 2, (void,), (u32le, 0x08074b50, yes), (void,), (void, 0, no))
bipa_struct(data_descriptor, 4
        , (union, data_descriptor_maybe_signature), _
        , (u32le,), crc_32
        , (u32le,), compressed_size
        , (u32le,), uncompressed_size
        )
// only if bit 3 of the general purpose bit flags
// "The correct values are put in the data descriptor immediately following the compressed data."
// TODO: but then how that works? how do I find the data_descriptor if it is past the file_data?
bipa_array(maybe_data_descriptor, (struct, data_descriptor))

bipa_struct(local_file, 3
        , (struct, local_file_header), local_file_header
        //, (struct, encryption_header), encryption_header // only if encrypted
        , (lstr, self->local_file_header.compressed_size), file_data
        , (array, maybe_data_descriptor, k==! (1<<3 & self->local_file_header.general_purpose_bit_flag)), data_descriptor
        )
bipa_array(local_files, (struct, local_file))

bipa_union(central_directory_header_signature, 1, (void,), (u32le, 0x02014b50, _))
bipa_struct(central_directory_header, 20
        , (union, central_directory_header_signature), _
        , (u16le,), version_made_by
        , (u16le,), version_needed_to_extract
        , (u16le,), general_purpose_bit_flag
        , (u16le,), compression_method
        , (u16le,), last_mod_file_time
        , (u16le,), last_mod_file_date
        , (u32le,), crc_32
        , (u32le,), compressed_size
        , (u32le,), uncompressed_size
        , (u16le,), file_name_length
        , (u16le,), extra_field_length
        , (u16le,), file_comment_length
        , (u16le,), disk_number_start
        , (u16le,), internal_file_attributes
        , (u32le,), external_file_attributes
        , (u32le,), relative_offset_of_local_header
        , (lstr, self->file_name_length), file_name
        , (lstr, self->extra_field_length), extra_field
        , (lstr, self->file_comment_length), file_comment
        )
bipa_array(central_directory_headers, (struct, central_directory_header))

bipa_struct(extra_field_header, 2
        , (u16le,), id
        , (u16le,), size
        )
bipa_struct(extra_field, 2
        , (struct, extra_field_header), header
        , (lstr, self->header.size), data
        )
bipa_array(extra_fields, (struct, extra_field))
bipa_struct(extra_fields_wrap, 1 , (array, extra_fields, *at < src->len), w)

bipa_union(digital_signature_header_signature, 1, (void,), (u32le, 0x05054b50, _))
bipa_struct(digital_signature, 3
        , (union, digital_signature_header_signature), _
        , (u16le,), size_of_data
        , (lstr, self->size_of_data), signature_data
        )

bipa_union(end_of_central_dir_signature, 1, (void,), (u32le, 0x06054b50, _))
bipa_struct(end_of_central_directory_record, 9
        , (union, end_of_central_dir_signature), _
        , (u16le,), number_of_this_disk
        , (u16le,), number_of_the_disk_with_the_start_of_the_central_directory
        , (u16le,), total_number_of_entries_in_the_central_directory_on_this_disk
        , (u16le,), total_number_of_entries_in_the_central_directory
        , (u32le,), size_of_the_central_directory
        , (u32le,), offset_of_start_of_central_directory_with_respect_to_the_starting_disk_number
        , (u16le,), zip_file_comment_length
        , (lstr, self->zip_file_comment_length), zip_file_comment
        )

bipa_struct(zip_data, 3
        , (array, local_files, !memcmp(src->ptr+*at, "\x50\x4b\x03\x04", 4)), local_files
        , (array, central_directory_headers, !memcmp(src->ptr+*at, "\x50\x4b\x01\x02", 4)), central_directory_headers
        , (struct, end_of_central_directory_record), end_of_central_directory_record
        )

/// https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT
adapt_bipa_type(zip_data)
buf zip_data_find_by_path(zip_data cref zip, buf cref path);

#ifdef BDF_IMPLEMENTATION

buf zip_data_find_by_path(zip_data cref zip, buf cref path) {
    (void)zip;
    (void)path;
    return (buf){0};
}

#endif // BDF_IMPLEMENTATION
