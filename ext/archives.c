/// https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT

#include "../helper.h"

export_names
    ( "ZipBuild"
    , "ZipParse"
    );

typedef struct ZipData {
    int plho;
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

    (void)source;

    fail("NIY: ZipParse");
}
