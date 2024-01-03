#include <stdio.h>

#define BIPA_HIDUMP
#include "../src/bipa.h"

bipa_struct(local_file_header, 2,
    (u16le), compression_method,
    (u16le), last_mod_file_time
)
bipa_struct(data_descriptor, 2,
    (u32le), crc_32,
    (u32le), compressed_size
)
bipa_array(local_file_headers, (struct, local_file_header))
bipa_struct(local_file, 2,
    (u16le), local_file_count,
    (array, local_file_headers, k < self->local_file_count), local_file_headers
    //(struct, data_descriptor), data_descriptor
)

void xxd(Buf const* const b) {
    for (sz k = 0; k < b->len; k++) {
        if (k && !(k & 0xf)) printf("\n");
        printf("%02X ", b->ptr[k]);
    }
}

int main(void) {
    struct local_file const src = {
        .local_file_count= 2,
        .local_file_headers= {
            .ptr= (struct local_file_header[2]){
                [0]= {.compression_method= 1, .last_mod_file_time= 1234},
                [1]= {.compression_method= 1, .last_mod_file_time= 5678},
            },
            .len= 2,
        },
    };
    struct local_file dst;

    printf("src:\n"); bipa_dump_local_file(&src, 0); printf("\n");

    BufBuilder builder = {0};
    bipa_build_local_file(&src, &builder);

    Buf b = {.len= builder.arr.len, .ptr= builder.arr.ptr};
    printf("buf:\n"); xxd(&b); printf("\n");

    BufParser parser = {.buf= &b};
    bipa_parse_local_file(&dst, &parser);

    free(builder.arr.ptr);

    printf("dst:\n"); bipa_dump_local_file(&dst, 0); printf("\n");
}
