#include <stdlib.h>

void text_init(void);
void text_draw(char const* txt, size_t len, float scale, int x, int y);
//void text_draw_u32(uint32_t const* txt, size_t len, float scale, int x, int y);
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
    glGenTextures(1, &_text_gl_tex);
    glBindTexture(GL_TEXTURE_2D, _text_gl_tex);

    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D,
            0, GL_ALPHA,
            128*8, _text_allf8x8_count*8,
            0,
            GL_ALPHA,
            GL_UNSIGNED_BYTE,
            0);

    for (size_t k = 0; k < _text_allf8x8_count; k++) {
        struct _f8x8 const it = _text_allf8x8[k];
        uint32_t count = it.cp_to - it.cp_from;

        uint8_t data[128*8*8] = {0};
        for (size_t ch = 0; ch < count; ch++)
            for (size_t j = 0; j < 8; j++)
                for (size_t i = 0; i < 8; i++)
                    data[ch*8 + i + count*j*8] = ((it.data[ch*8 + 7-j] >> i) & 1) * 255;

        glTexSubImage2D(GL_TEXTURE_2D,
                0,
                0, k*8,
                count*8, 8,
                GL_ALPHA,
                GL_UNSIGNED_BYTE,
                data);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

void text_draw(char const* txt, size_t len, float scale, int x, int y) {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, _text_gl_tex);

    glBegin(GL_QUADS);

    size_t cx = 0, cy = 0;
    for (size_t k = 0; k < len; k++) {
        uint32_t u = txt[k];

        if (0 == (0b10000000 & u))
            ;
        else if (0 == (0b00100000 & u) && k+1 < len) {
            char x = txt[++k];
            u = ((u & 0b00011111) << 6) | (x & 0b00111111);
        }
        else if (0 == (0b00010000 & u) && k+2 < len) {
            char x = txt[++k], y = txt[++k];
            u = ((u & 0b00001111) << 12) | ((x & 0b00111111) << 6) | (y & 0b00111111);
        }
        else if (0 == (0b00001000 & u) && k+3 < len) {
            char x = txt[++k], y = txt[++k], z = txt[++k];
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
        if (!found) { u = off = '?'; k = 0; }

        glTexCoord2f((float)(off+0)/128, (float)(k+0)/_text_allf8x8_count); glVertex2f(x + cx*8*scale + 0*scale, y + cy*8*scale + 0*scale);
        glTexCoord2f((float)(off+0)/128, (float)(k+1)/_text_allf8x8_count); glVertex2f(x + cx*8*scale + 0*scale, y + cy*8*scale + 8*scale);
        glTexCoord2f((float)(off+1)/128, (float)(k+1)/_text_allf8x8_count); glVertex2f(x + cx*8*scale + 8*scale, y + cy*8*scale + 8*scale);
        glTexCoord2f((float)(off+1)/128, (float)(k+0)/_text_allf8x8_count); glVertex2f(x + cx*8*scale + 8*scale, y + cy*8*scale + 0*scale);

        switch (u) {
            case '\b': if (cx) cx--; break;
            case '\t': cx = ((cx/4) + 1)*4; break;
            case '\n': if (cy) cy--; break;
            case '\r': cx = 0; break;
            default: cx++;
        }
    }

    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void text_free(void) {
    glDeleteTextures(1, &_text_gl_tex);
    _text_gl_tex = 0;
}

#undef _text_allf8x8_count
#endif // TEXT_IMPLEMENTATION
