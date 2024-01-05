#include <stdio.h>

#define BIPA_HIDUMP
#include "../src/bipa.h"

void xxd(Buf const* const b) {
    for (sz k = 0; k < b->len; k++) {
        if (k && !(k & 0xf)) printf("\n");
        printf("%02X ", b->ptr[k]);
    }
}

#define roundtrip(__tname, ...) do {                             \
        puts("# roundtrip for a " #__tname);                     \
        struct __tname const src = __VA_ARGS__;                  \
                                                                 \
        puts("src:"); bipa_dump_##__tname(&src, 0); puts("");    \
                                                                 \
        BufBuilder builder = {0};                                \
        bipa_build_##__tname(&src, &builder);                    \
        Buf b = {.len= builder.arr.len, .ptr= builder.arr.ptr};  \
        puts("buf:"); xxd(&b); puts("");                         \
                                                                 \
        struct __tname res;                                      \
        BufParser parser = {.buf= &b};                           \
        bipa_parse_##__tname(&res, &parser);                     \
        free(b.ptr);                                             \
                                                                 \
        puts("res:"); bipa_dump_##__tname(&res, 0); puts("");    \
                                                                 \
        bipa_free_##__tname(&res);                               \
        puts("");                                                \
    } while (0)

bipa_struct(test_cstr, 1
        , (cstr, '\0'), data
        )

bipa_struct(test_lstr, 2
        , (u16be), length
        , (lstr, self->length), data
        )

bipa_union(test_union, 4
        , (void), (u8, 42, nothing)
        , (u16le), (u8, 48, ushort)
        , (u32le), (u8, 49, uint)
        , (u64le), (u8, 50, ulong)
        )

bipa_union(maybe_u64, 2
        , (u64le), (u32le, 0x08074b50, yes)
        , (u64le), (void, 0, no)
        )

bipa_array(test_lines, (cstr, '\n'))
bipa_struct(test_text, 2
        , (u32be), line_count
        , (array, test_lines, k < self->line_count), lines
        )

int main(void) {
    if(1) roundtrip(test_cstr, {.data= (u8*)"hi :3"});
    if(1) roundtrip(test_lstr, {.length= 5, .data= (u8*)"hi :3"});
    if(1) roundtrip(test_union, {
        .val.uint= 42,
        //.tag= test_union_tag_uint,
        .tag= test_union_tag_nothing,
    });
    if(1) roundtrip(maybe_u64, {
        .val= {9876543210},
        //.tag= maybe_tag_yes,
        .tag= maybe_u64_tag_no,
    });
    if(1) roundtrip(test_text, {
        .line_count= 4,
        .lines= {
            .len= 4,
            .ptr= (u8*[]){
                (u8*)"one\n",
                (u8*)"two\n",
                (u8*)"three\n",
                (u8*)"need a line thats like x16 chars\n",
            },
        },
    });
    return 0;
}
