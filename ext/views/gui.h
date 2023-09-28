///
/// _text_rect can be `#define`d to a custom function,
/// the default behavior should match with text.h for ASCII text.

#include <stdbool.h>
#include <stdlib.h>

#ifndef _text_rect
void _text_rect(char const* txt, size_t len, float scale, size_t* w, size_t* h);
#endif

#ifndef GUI_IMPLEMENTATION
#include <stdint.h>

#ifndef _text_rect
void _text_rect(char const* txt, size_t len, float scale, size_t* w, size_t* h) {
    *w = *h = 0;

    size_t cx = 0, cy = 0;
    for (size_t k = 0; k < len; k++) {
        uint32_t u = txt[k];
        switch (u) {
            case '\b': cx--; break;
            case '\t': cx = ((cx/4) + 1)*4; break;
            case '\n': cy--; break;
            case '\r': cx = 0; break;
            default: cx++;
        }

        if (*w < cx) *w = cx;
        if (*h < cy) *h = cy;
    }

    *w*= 8*scale;
    *h*= 8*scale;
}
#endif

#endif // GUI_IMPLEMENTATION
