#include <stdio.h>
#include <stdlib.h>

#define BIPA_HIDUMP
#include "../src/bipa.h"

bipa_array(array_b, (u8));
bipa_array(object_b, (u8));
bipa_union(json, 7
        , (void), (u32be, 0x6e756c6c, null)
        , (void), (u32be, 0x74727565, boolean_true)
        , (u8  ), (u32be, 0x66616c73, boolean_false)
        , (cstr, '"'), (u8, '"', string)
        , (array, array_b, ']' != pa->buf->ptr[pa->at]), (u8, '[', array)
        , (array, object_b, '}' != pa->buf->ptr[pa->at]), (u8, '{', object)
        , (void), (void, 0, number)
        )

/*
bipa_array(array, (struct, json));

bipa_struct(pair, 4
        , (u8), _quote
        , (cstr, '"'), key
        , (u8), _colon
        , (struct, json), value
        )
bipa_array(object, (struct, pair));
*/

void xxd(Buf const* const b) {
    for (sz k = 0; k < b->len; k++) {
        if (k && !(k & 0xf)) printf("\n");
        printf("%02X ", b->ptr[k]);
    }
}

int main(int argc, char** argv) {
    if (argc <2) return puts("missing file argument");

    Buf b;
    {
        FILE* f = fopen(argv[1], "r");
        if (!f) return puts("could not read file");

        fseek(f, 0, SEEK_END);
        b.ptr = malloc(b.len = ftell(f));
        fseek(f, 0, SEEK_SET);
        fread(b.ptr, 1, b.len, f);

        fclose(f);
    }

    xxd(&b);
    puts("---");

    struct json r;
    {
        BufParser p = {.buf= &b};
        bipa_parse_json(&r, &p);
        free(b.ptr);
    }

    bipa_dump_json(&r, 0);
    puts("");

    bipa_free_json(&r);

    return 0;
}
