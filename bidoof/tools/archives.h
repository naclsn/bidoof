#ifndef __BIDOOF_T_ARCHIVES__
#define __BIDOOF_T_ARCHIVES__

#ifdef BIDOOF_T_IMPLEMENTATION
#define _redef_after_archives
#undef BIDOOF_IMPLEMENTATION
#undef BIDOOF_T_IMPLEMENTATION
#endif
#include "checks.h"
#ifdef _redef_after_archives
#undef _redef_after_archives
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
static struct _list_deps_item const _list_deps_me_archives = {_list_deps_first, "archives"};
#undef _list_deps_first
#define _list_deps_first &_list_deps_me_archives
#endif

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

// https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT
adapt_bipa_type(zip_data)

typedef struct zip_entry {
    struct zip_data* zip;
    struct local_file* file;
    struct central_directory_header* header;
} zip_entry;

buf zip_find_by_path(zip_data cref zip, buf const path);

zip_entry zip_add_entry(zip_data ref zip);
void zip_entry_file_name(zip_entry ref en, buf const file_name);
void zip_entry_extra_field(zip_entry ref en, unsigned const n, u16 const ids[n], buf const data[n]);
void zip_entry_file_data(zip_entry ref en, buf const file_data);
void zip_fix_entry(zip_entry ref en);

void zip_fix_central_dir(zip_data ref zip);
void zip_central_dir_comment(zip_data ref zip, buf const com);

#ifdef BIDOOF_IMPLEMENTATION

buf zip_find_by_path(zip_data cref zip, buf const path) {
    (void)zip;
    (void)path;
    return (buf){0};
}

zip_entry zip_add_entry(zip_data ref zip) {
    struct local_file* file = dyarr_push(&zip->local_files);
    struct central_directory_header* header = dyarr_push(&zip->central_directory_headers);
    if (!file || !header) exitf("OOM");

    file->local_file_header.version_needed_to_extract = 10;
    header->version_needed_to_extract = 10;

    return (zip_entry){.zip= zip, .file= file, .header= header};
}

void zip_entry_file_name(zip_entry ref en, buf const file_name) {
    en->file->local_file_header.file_name_length = file_name.len;
    en->file->local_file_header.file_name = bufcpy(file_name).ptr;
    en->header->file_name_length = file_name.len;
    en->header->file_name = bufcpy(file_name).ptr;
}

void zip_entry_extra_field(zip_entry ref en, unsigned const n, u16 const ids[n], buf const data[n]) {
    struct extra_field p[n];
    struct extra_fields_wrap f = {.w= {.len= n, .ptr= p}};

    for (unsigned k = 0; k < n; k++)
        f.w.ptr[k] = (struct extra_field){
            .header= (struct extra_field_header){
                .id= ids[k],
                .size= data[k].len,
            },
            .data= data[k].ptr,
        };

    buf b = {0};
    if (bipa_build_extra_fields_wrap(&f, &b)) {
        en->file->local_file_header.extra_field_length = b.len;
        en->file->local_file_header.extra_field = bufcpy(b).ptr;
        en->header->extra_field_length = b.len;
        en->header->extra_field = bufcpy(b).ptr;
    }
    buf_free(b);
}

void zip_entry_file_data(zip_entry ref en, buf const file_data) {
    en->file->local_file_header.compressed_size = file_data.len;
    en->file->file_data = bufcpy(file_data).ptr;
}

void zip_fix_entry(zip_entry ref en) {
    u32 crc = crc32(mkbufsl(en->file->file_data, 0, en->file->local_file_header.compressed_size));
    en->file->local_file_header.crc_32 = crc;
    en->header->crc_32 = crc;

    // for now
    en->file->local_file_header.uncompressed_size =
    en->header->compressed_size =
    en->header->uncompressed_size =
        en->file->local_file_header.compressed_size;

    u32 off = 0;
    for (struct local_file const* it = en->zip->local_files.ptr; it != en->file; it++)
        off+= bipa_bytesz_local_file(it);
    en->header->relative_offset_of_local_header = off;
}

void zip_fix_central_dir(zip_data ref zip) {
    struct end_of_central_directory_record* const d = &zip->end_of_central_directory_record;

    d->number_of_this_disk = 0;
    d->number_of_the_disk_with_the_start_of_the_central_directory = 0;

    d->total_number_of_entries_in_the_central_directory_on_this_disk = zip->local_files.len;
    d->total_number_of_entries_in_the_central_directory = zip->local_files.len;

    d->size_of_the_central_directory =
        bipa_bytesz_central_directory_headers(&zip->central_directory_headers);

    d->offset_of_start_of_central_directory_with_respect_to_the_starting_disk_number =
        bipa_bytesz_local_files(&zip->local_files);
}

void zip_central_dir_comment(zip_data ref zip, buf const com) {
    struct end_of_central_directory_record* const d = &zip->end_of_central_directory_record;

    free(d->zip_file_comment);

    d->zip_file_comment_length = com.len;
    d->zip_file_comment = bufcpy(com).ptr;
}

#endif // BIDOOF_IMPLEMENTATION

#endif // __BIDOOF_T_ARCHIVES__
