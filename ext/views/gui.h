///
/// The following things need to exist (values/functions/macros/..):
/// - `void text_draw(char const* txt, size_t len, int x, int y);`
/// - `int MOUSE_LEFT, MOUSE_RIGHT, MOUSE_MIDDLE`
///
/// Optionally, text_area can be `#define`d to a custom function.
/// The default behavior should match with text.h for ASCII text.

#include <stdbool.h>
#include <stdlib.h>

struct _GuiState {
    int width, height;
    float scale;
    struct {
        bool need_redraw :1;
        bool will_need_redraw :1;
        bool mouse_event :1;
    } flags;
    struct {
        int x, y;
        struct {
            bool left :1;
            bool right :1;
            bool middle :1;
        } buttons;
    } mouse;
};

void gui_begin(struct _GuiState* st);
void gui_end(struct _GuiState* st);
#define gui_need_redraw(__st)  ((__st)->flags.need_redraw)

void gui_event_reshape(struct _GuiState* st, int w, int h, float scale);
void gui_event_mousedown(struct _GuiState* st, int button);
void gui_event_mouseup(struct _GuiState* st, int button);
void gui_event_mousemove(struct _GuiState* st, int x, int y);

void gui_text(struct _GuiState* st, char const* text);

enum _GuiButtonState {
    BUTTON_RESTING,
    BUTTON_HOVERED,
    BUTTON_PRESSED,
    BUTTON_RELEASED,
    BUTTON_DISABLED,
};
struct _GuiButton {
    char const* text;
    enum _GuiButtonState state;
};
void gui_button(struct _GuiState* st, struct _GuiButton* self);

#ifdef GUI_IMPLEMENTATION
#include <stdint.h>
#include <string.h>
#include <GL/gl.h>

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
    if (!st->scale) st->scale = 1;
    if (st->flags.need_redraw) {
        glClearColor(.12f, .15f, .18f, 1);
        glClear(GL_COLOR_BUFFER_BIT);
    }
}
void gui_end(struct _GuiState* st) {
    if (st->flags.will_need_redraw) {
        st->flags.will_need_redraw = false;
        st->flags.need_redraw = true;
    } else if (st->flags.need_redraw)
        st->flags.need_redraw = false;
    st->flags.mouse_event = false;
}

void gui_event_reshape(struct _GuiState* st, int w, int h, float scale) {
    st->width = w;
    st->height = h;
    st->scale = scale;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, w/scale, h/scale, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    st->flags.will_need_redraw = true;
}

void gui_event_mousedown(struct _GuiState* st, int button) {
    if (MOUSE_LEFT   == button) st->mouse.buttons.left   = true;
    if (MOUSE_RIGHT  == button) st->mouse.buttons.right  = true;
    if (MOUSE_MIDDLE == button) st->mouse.buttons.middle = true;
    st->flags.mouse_event = true;
}
void gui_event_mouseup(struct _GuiState* st, int button) {
    if (MOUSE_LEFT   == button) st->mouse.buttons.left   = false;
    if (MOUSE_RIGHT  == button) st->mouse.buttons.right  = false;
    if (MOUSE_MIDDLE == button) st->mouse.buttons.middle = false;
    st->flags.mouse_event = true;
}
void gui_event_mousemove(struct _GuiState* st, int x, int y) {
    st->mouse.x = x;
    st->mouse.y = y;
    st->flags.mouse_event = true;
}

void gui_text(struct _GuiState* st, char const* text) {
    struct _GuiRect r;

    text_area(text, strlen(text), &r.width, &r.height);
    r.x = r.y = 16;

    if (st->flags.need_redraw) {
        glColor4f(1, .4, .7, 1);
        text_draw(text, strlen(text), r.x, r.y);
    }
}

void gui_button(struct _GuiState* st, struct _GuiButton* self) {
    struct _GuiRect r;

    text_area(self->text, strlen(self->text), &r.width, &r.height);
    r.x = r.y = 8;

    if (st->flags.need_redraw) {
        switch (self->state) {
            case BUTTON_RESTING:  glColor4f(.88, .66, .77, 1); break;
            case BUTTON_HOVERED:  glColor4f(  1, .88, .99, 1); break;
            case BUTTON_PRESSED:  glColor4f(.66, .44, .55, 1); break;
            case BUTTON_RELEASED: glColor4f(.88, .66, .77, 1); break;
            case BUTTON_DISABLED: glColor4f(.44, .22, .33, 1); break;
        }

        text_draw(self->text, strlen(self->text), r.x, r.y);
    } else {
        bool is_in = rect_in(&r, st->mouse.x/st->scale, st->mouse.y/st->scale);
        bool is_down = st->flags.mouse_event && st->mouse.buttons.left;
        bool is_up = st->flags.mouse_event && !st->mouse.buttons.left;

        enum _GuiButtonState const pstate = self->state;
        switch (self->state) {
            case BUTTON_RESTING:
                if (is_in) self->state = BUTTON_HOVERED;
                break;
            case BUTTON_HOVERED:
                if (!is_in) self->state = BUTTON_RESTING;
                else if (is_down) self->state = BUTTON_PRESSED;
                break;
            case BUTTON_PRESSED:
                if (!is_in) self->state = BUTTON_RESTING;
                else if (is_up) self->state = BUTTON_RELEASED;
                break;
            case BUTTON_RELEASED:
                if (is_in && is_down) self->state = BUTTON_PRESSED;
                else if (is_in) self->state = BUTTON_HOVERED;
                else self->state = BUTTON_RESTING;
                break;
            case BUTTON_DISABLED:
                break;
        }

        if (pstate != self->state) st->flags.will_need_redraw = true;
    } // else !need_redraw
}

#endif // GUI_IMPLEMENTATION
