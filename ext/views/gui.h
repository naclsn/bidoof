///
/// The following things need to exist (either functions or macros):
/// - `void text_draw(char const* txt, size_t len, int x, int y);`
///
/// Optionally, text_area can be `#define`d to a custom function.
/// The default behavior should match with text.h for ASCII text.

#include <stdbool.h>
#include <stdlib.h>

#define gui_state(__idt)  \
    static struct _GuiState __idt = {.scale= 1};
struct _GuiState {
    int width, height;
    float scale;
    struct {
        int x, y;
    } mouse;
};

void gui_event_reshape(struct _GuiState* st, int w, int h, float scale);
void gui_event_mousedown(struct _GuiState* st, int button, int x, int y);
void gui_event_mousemove(struct _GuiState* st, int x, int y);

#define gui_button(__idt, __state_ptr, __txt, __enabled)  \
    static struct _GuiButton __idt = {0};                 \
    _gui_button(&__idt, __state_ptr, __txt, __enabled)
struct _GuiButton {
    bool pressed;
};
void _gui_button(struct _GuiButton* self, struct _GuiState* st, char const* text, bool enabled);

#ifdef GUI_IMPLEMENTATION
#include <stdint.h>

#ifndef text_area
void text_area(char const* txt, size_t len, int* w, int* h) {
    *w = *h = 0;

    int cx = 0, cy = 1;
    for (size_t k = 0; k < len; k++) {
        uint32_t u = txt[k];
        switch (u) {
            case '\b': if (cx) cx--; break;
            case '\t': cx = ((cx/4) + 1)*4; break;
            case '\n': cy++; break;
            case '\r': cx = 0; break;
            default: cx++;
        }

        if (*w < cx) *w = cx;
        if (*h < cy) *h = cy;
    }

    *w*= 8;
    *h*= 8;
}
#endif

struct _GuiRect { int x, y, width, height; };
static inline bool rect_in(struct _GuiRect* r, int x, int y)
{ return r->x <= x && x < r->x+r->width && r->y <= y && y < r->y+r->height; }

void gui_event_reshape(struct _GuiState* st, int w, int h, float scale) {
    st->width = w;
    st->height = h;
    st->scale = scale;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, w/scale, h/scale, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
}

void gui_event_mousedown(struct _GuiState* st, int button, int x, int y) {
    (void)st;
    (void)button;
    st->mouse.x = x;
    st->mouse.y = y;
}

void gui_event_mousemove(struct _GuiState* st, int x, int y) {
    st->mouse.x = x;
    st->mouse.y = y;
}

void _gui_button(struct _GuiButton* self, struct _GuiState* st, char const* text, bool enabled) {
    struct _GuiRect r;

    r.x = r.y = 8;
    text_area(text, strlen(text), &r.width, &r.height);

    if (enabled) {
        if (rect_in(&r, st->mouse.x/st->scale, st->mouse.y/st->scale))
             glColor4f(1, 1, 1, 1);
        else glColor4f(.88f, .66f, .77f, 1.f);
    } else   glColor4f(.44f, .22f, .33f, 1.f);

    text_draw(text, strlen(text), r.x, r.y);

    (void)self;
    //self->pressed = false;
}

#endif // GUI_IMPLEMENTATION
