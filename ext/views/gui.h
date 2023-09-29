///
/// The following things need to exist (values/functions/macros/..):
/// - `void text_draw(char const* txt, size_t len, int x, int y);`
/// - `int MOUSE_LEFT, MOUSE_RIGHT, MOUSE_MIDDLE`
///
/// Optionally, text_area can be `#define`d to a custom function.
/// The default behavior should match with text.h for ASCII text.

#include <stdbool.h>
#include <stdlib.h>

#define gui_state(__idt)  \
    static struct _GuiState __idt = {.scale= 1, .flags= GUIST_NEED_REDRAW};
struct _GuiState {
    int width, height;
    float scale;
    enum {
        GUIST_NEED_REDRAW      = 0b00000001,
        GUIST_WILL_NEED_REDRAW = 0b00000010,
        GUIST_MOUSE_EVENT      = 0b00000100,
    } flags;
    struct {
        int x, y;
        enum {
            GUIM_LEFT   = 0b001,
            GUIM_RIGHT  = 0b010,
            GUIM_MIDDLE = 0b100,
        } buttons;
    } mouse;
};

void gui_begin(struct _GuiState* st);
void gui_end(struct _GuiState* st);
#define gui_need_redraw(__st)  (GUIST_NEED_REDRAW & (__st)->flags)

void gui_event_reshape(struct _GuiState* st, int w, int h, float scale);
void gui_event_mousedown(struct _GuiState* st, int button);
void gui_event_mouseup(struct _GuiState* st, int button);
void gui_event_mousemove(struct _GuiState* st, int x, int y);

#define gui_button(__st, __idt, __txt, __enabled)  \
    static struct _GuiButton __idt = {0};          \
    _gui_button(__st, &__idt, __txt, __enabled)
struct _GuiButton {
    bool pressed;
    bool released;
    bool _was_in;
};
void _gui_button(struct _GuiState* st, struct _GuiButton* self, char const* text, bool enabled);

#ifdef GUI_IMPLEMENTATION
#include <stdint.h>
#include <string.h>

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

void gui_begin(struct _GuiState* st) {
    if (GUIST_NEED_REDRAW & st->flags) {
        glClearColor(.12f, .15f, .18f, 1);
        glClear(GL_COLOR_BUFFER_BIT);
    }
}
void gui_end(struct _GuiState* st) {
    if (GUIST_WILL_NEED_REDRAW & st->flags)
        st->flags = (st->flags &~ GUIST_WILL_NEED_REDRAW) | GUIST_NEED_REDRAW;
    else if (GUIST_NEED_REDRAW & st->flags)
        st->flags&=~GUIST_NEED_REDRAW;
    st->flags&=~GUIST_MOUSE_EVENT;
}

void gui_event_reshape(struct _GuiState* st, int w, int h, float scale) {
    st->width = w;
    st->height = h;
    st->scale = scale;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, w/scale, h/scale, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    st->flags|= GUIST_WILL_NEED_REDRAW;
}

void gui_event_mousedown(struct _GuiState* st, int button) {
    st->mouse.buttons|=
            ( MOUSE_LEFT   == button ? GUIM_LEFT
            : MOUSE_RIGHT  == button ? GUIM_RIGHT
            : MOUSE_MIDDLE == button ? GUIM_MIDDLE
            : 0 );
    st->flags|= GUIST_MOUSE_EVENT;
}
void gui_event_mouseup(struct _GuiState* st, int button) {
    st->mouse.buttons&=~
            ( MOUSE_LEFT   == button ? GUIM_LEFT
            : MOUSE_RIGHT  == button ? GUIM_RIGHT
            : MOUSE_MIDDLE == button ? GUIM_MIDDLE
            : 0 );
    st->flags|= GUIST_MOUSE_EVENT;
}
void gui_event_mousemove(struct _GuiState* st, int x, int y) {
    st->mouse.x = x;
    st->mouse.y = y;
    st->flags|= GUIST_MOUSE_EVENT;
}

void _gui_button(struct _GuiState* st, struct _GuiButton* self, char const* text, bool enabled) {
    struct _GuiRect r;

    r.x = r.y = 8;
    text_area(text, strlen(text), &r.width, &r.height);
    bool is_in = rect_in(&r, st->mouse.x/st->scale, st->mouse.y/st->scale);

    if (GUIST_NEED_REDRAW & st->flags) {
        if (enabled) {
            if (is_in)
                 glColor4f(1, 1, 1, 1);
            else glColor4f(.88f, .66f, .77f, 1.f);
        } else   glColor4f(.44f, .22f, .33f, 1.f);

        text_draw(text, strlen(text), r.x, r.y);
    } else if (is_in != self->_was_in) {
        self->_was_in = is_in;
        st->flags|= GUIST_WILL_NEED_REDRAW;
    }

    //printf("enabled: %s, is_in: %s, mouse_event: %s, mouse_left: %s\n",
    //        enabled ? "true" : "false",
    //        is_in ? "true" : "false",
    //        GUIST_MOUSE_EVENT & st->flags ? "true" : "false",
    //        GUIM_LEFT == st->mouse.buttons ? "true" : "false");

    bool is_pressed = enabled
                && is_in
                && GUIST_MOUSE_EVENT & st->flags
                && GUIM_LEFT & st->mouse.buttons;
    bool was_pressed = self->pressed;
    self->pressed = !was_pressed && is_pressed;
    self->released = was_pressed && !is_pressed;
}

#endif // GUI_IMPLEMENTATION
