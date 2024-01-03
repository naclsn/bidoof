#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define sz size_t
typedef struct Buf { sz len; u8* ptr; } Buf;

#define _BIPA_HIDUMP
#include "bipa.h"

bipa_struct(local_file_header,
    (u16le), compression_method,
    (u16le), last_mod_file_time
)
bipa_struct(data_descriptor,
    (u32le), crc_32,
    (u32le), compressed_size
)
bipa_array(local_file_headers, (struct, local_file_header))
bipa_struct(local_file,
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

    printf("src: "); bipa_dump_local_file(&src); printf("\n");

    BufBuilder builder = {0};
    bipa_build_local_file(&src, &builder);

    Buf b = {.len= builder.arr.len, .ptr= builder.arr.ptr};
    printf("buf:\n"); xxd(&b); printf("\n");

    BufParser parser = {.buf= &b};
    bipa_parse_local_file(&dst, &parser);

    free(builder.arr.ptr);

    printf("dst: "); bipa_dump_local_file(&dst); printf("\n");
}
