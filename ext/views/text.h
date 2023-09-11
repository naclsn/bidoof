#include <stdlib.h>

void text_init(void);
void text_draw(char const* c, size_t l, int x, int y, float s);
//void text_draw_u32(uint32_t const* c, size_t l, int x, int y, float s);
void text_free(void);

#ifdef TEXT_IMPLEMENTATION
#include <stdbool.h>
#include <stdint.h>
#include <GL/gl.h>

#include "font8x8/font8x8.h"

static struct _f8x8 {
    char const* const data;
    uint32_t const cp_from;
    uint32_t const cp_to;
} const _text_allf8x8[] = {
    {(char*)font8x8_basic,        0x0,   0x7f},
    {(char*)font8x8_control,     0x80,   0x9f},
    {(char*)font8x8_ext_latin,   0xa0,   0xff},
    {(char*)font8x8_greek,      0x390,  0x3c9},
    {(char*)font8x8_box,       0x2500, 0x257f},
    {(char*)font8x8_block,     0x2580, 0x259f},
    {(char*)font8x8_hiragana,  0x3040, 0x309f},
};
#define _text_allf8x8_count (sizeof(_text_allf8x8) / sizeof(_text_allf8x8[0]))
static GLuint _text_gl_tex;

void text_init(void) {
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &_text_gl_tex);
    glBindTexture(GL_TEXTURE_2D, _text_gl_tex);

    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D,
            0, GL_RGBA,
            128*8, _text_allf8x8_count*8,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            0);

    for (size_t k = 0; k < _text_allf8x8_count; k++) {
        struct _f8x8 const it = _text_allf8x8[k];
        uint32_t count = it.cp_to - it.cp_from;

        uint8_t bata[128*8*8*4] = {0};
        for (size_t ch = 0; ch < count; ch++) {
            for (size_t j = 0; j < 8; j++) {
                for (size_t i = 0; i < 8; i++) {
                    size_t at = ch*8 + i + count*j*8;
                    uint8_t v = ((it.data[ch*8 + 7-j] >> i) & 1) * 255;
                    bata[at*4+0] = 255;
                    bata[at*4+1] = 255;
                    bata[at*4+2] = 255;
                    bata[at*4+3] = v;
                }
            }
        }

        glTexSubImage2D(GL_TEXTURE_2D,
                0,
                0, k*8,
                count*8, 8,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                bata);
    }
}

void text_allf8x8(void) {
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(1.f/128, 1.f/_text_allf8x8_count, 1.f);
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    {
        float s = 2;
        glTexCoord2f(0, 0); glVertex2f(0, 0);
        glTexCoord2f(0, _text_allf8x8_count); glVertex2f(0, _text_allf8x8_count*s);
        glTexCoord2f(128, _text_allf8x8_count); glVertex2f(128*s, _text_allf8x8_count*s);
        glTexCoord2f(128, 0); glVertex2f(128*s, 0);
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

void text_draw(char const* c, size_t l, int x, int y, float s) {
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(1.f/128, 1.f/_text_allf8x8_count, 1.f);
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    int cx = 0, cy = 0;
    for (size_t k = 0; k < l; k++) {
        uint32_t u = c[k];

        if (0 == (0b10000000 & u))
            ;
        else if (0 == (0b00100000 & u) && k+1 < l) {
            char x = c[++k];
            u = ((u & 0b00011111) << 6) | (x & 0b00111111);
        }
        else if (0 == (0b00010000 & u) && k+2 < l) {
            char x = c[++k], y = c[++k];
            u = ((u & 0b00001111) << 12) | ((x & 0b00111111) << 6) | (y & 0b00111111);
        }
        else if (0 == (0b00001000 & u) && k+3 < l) {
            char x = c[++k], y = c[++k], z = c[++k];
            u = ((u & 0b00000111) << 18) | ((x & 0b00111111) << 12) | ((y & 0b00111111) << 6) | (z & 0b00111111);
        }
        else u = '?';

        size_t k;
        uint32_t off = 0;
        bool found = false;
        for (k = 0; k < _text_allf8x8_count; k++) {
            struct _f8x8 const it = _text_allf8x8[k];
            if (it.cp_from <= u && u <= it.cp_to) {
                off = u - _text_allf8x8[k].cp_from;
                found = true;
                break;
            }
        }
        if (!found) {
            u = '?';
            k = 0;
            off = u;
        }

        glTexCoord2f(off+0, k+0); glVertex2f(x + cx*8*s + 0*s, y + cy*8*s + 0*s);
        glTexCoord2f(off+0, k+1); glVertex2f(x + cx*8*s + 0*s, y + cy*8*s + 8*s);
        glTexCoord2f(off+1, k+1); glVertex2f(x + cx*8*s + 8*s, y + cy*8*s + 8*s);
        glTexCoord2f(off+1, k+0); glVertex2f(x + cx*8*s + 8*s, y + cy*8*s + 0*s);

        switch (u) {
            case '\t': cx = ((cx/4) + 1)*4; break;
            case '\n': cy--; break;
            case '\r': cx = 0; break;
            default: cx++;
        }
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

void text_free(void) {
    glDeleteTextures(1, &_text_gl_tex);
    _text_gl_tex = 0;
}

#undef _text_allf8x8_count
#endif // TEXT_IMPLEMENTATION
