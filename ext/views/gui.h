///
/// label
/// button
/// select
/// input
/// toggle
/// ~~image~~
///
/// layout_[vh]splits
/// layout_table
///
/// box/container/div/.. (scroll bar)
///
///
/// The following things need to exist (values/functions/macros/..):
/// - `void text_draw(char const* txt, size_t len, int x, int y);`
/// - `int const MOUSE_LEFT, MOUSE_RIGHT, MOUSE_MIDDLE;`
///
/// Optionally, text_area can be `#define`d to a custom function.
/// The default behavior should match with text.h.

#include <stdbool.h>

typedef struct GuiRect { float x, y, width, height; } GuiRect;

#define _extends_GuiLayoutBase  struct _GuiLayoutBase* parent; GuiRect rect
struct _GuiLayoutBase { _extends_GuiLayoutBase; };

typedef struct GuiState {
    _extends_GuiLayoutBase;

    struct _GuiLayoutBase* layout;

    float scale;

    struct {
        bool need_redraw :1;
        bool will_need_redraw :1;
        bool mouse_event :1;
    } flags;

    struct {
        float x, y;
        struct {
            bool left :1;
            bool right :1;
            bool middle :1;
        } buttons;
    } mouse;
} GuiState;

void gui_begin(GuiState* st);
void gui_end(GuiState* st);
#define gui_need_redraw(__st)  ((__st)->flags.need_redraw)

void gui_event_reshape(GuiState* st, int w, int h, float scale);
void gui_event_mousedown(GuiState* st, int button);
void gui_event_mouseup(GuiState* st, int button);
void gui_event_mousemove(GuiState* st, int x, int y);

typedef struct GuiLayoutFixed {
    _extends_GuiLayoutBase;
} GuiLayoutFixed;
void gui_layout_fixed_push(GuiState* st, GuiLayoutFixed* self);
void gui_layout_fixed_pop(GuiState* st, GuiLayoutFixed* self);

typedef struct GuiLayoutSplits {
    _extends_GuiLayoutBase;
    enum { SPLITS_VERTICAL, SPLITS_HORIZONTAL } direction;
    unsigned count, current;
} GuiLayoutSplits;
void gui_layout_splits_push(GuiState* st, GuiLayoutSplits* self);
void gui_layout_splits_next(GuiState* st, GuiLayoutSplits* self);
void gui_layout_splits_pop(GuiState* st, GuiLayoutSplits* self);

typedef struct GuiLabel {
    char const* text;
} GuiLabel;
void gui_label(GuiState* st, GuiLabel* text);

typedef enum GuiButtonState {
    BUTTON_RESTING,
    BUTTON_HOVERED,
    BUTTON_PRESSED,
    BUTTON_RELEASED,
    BUTTON_DISABLED,
} GuiButtonState;
typedef struct GuiButton {
    char const* text;
    GuiButtonState state;
} GuiButton;
void gui_button(GuiState* st, GuiButton* self);

typedef enum GuiMenuState {
    MENU_RESTING,
    MENU_OPENED,
    MENU_SELECTED,
} GuiMenuState;
typedef struct GuiMenu {
    GuiMenuState state;
    GuiLayoutFixed box;
    GuiLayoutSplits splits;
    unsigned count, pick;
    GuiButton choices[];
} GuiMenu;
void gui_menu(GuiState* st, GuiMenu* self);
void gui_menu_alt(GuiState* st, GuiMenu* self);

#ifdef GUI_IMPLEMENTATION
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>

#ifndef text_area
void text_area(char const* txt, size_t len, int* w, int* h) {
    *w = *h = 0;

    int cx = 0, cy = 1;
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

static inline bool rect_in(GuiRect const r, float x, float y)
{ return r.x <= x && x < r.x+r.width && r.y <= y && y < r.y+r.height; }
static inline GuiRect rect_pad(GuiRect const r, float dx, float dy)
{ return (GuiRect){.x= r.x-dx, .y= r.y-dy, .width= r.width+2*dx, .height= r.height+2*dy}; }

void gui_begin(GuiState* st) {
    if (!st->scale) st->scale = 1;
    st->layout = (struct _GuiLayoutBase*)st;
    if (st->flags.need_redraw) {
        glClearColor(.12f, .15f, .18f, 1);
        glClear(GL_COLOR_BUFFER_BIT);
    }
}
void gui_end(GuiState* st) {
    if (st->flags.will_need_redraw) {
        st->flags.will_need_redraw = false;
        st->flags.need_redraw = true;
    } else if (st->flags.need_redraw)
        st->flags.need_redraw = false;
    st->flags.mouse_event = false;
}

void gui_event_reshape(GuiState* st, int w, int h, float scale) {
    st->rect.width = w/scale;
    st->rect.height = h/scale;
    st->scale = scale;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, w/scale, h/scale, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    st->flags.will_need_redraw = true;
}

void gui_event_mousedown(GuiState* st, int button) {
    if (MOUSE_LEFT   == button) st->mouse.buttons.left   = true;
    if (MOUSE_RIGHT  == button) st->mouse.buttons.right  = true;
    if (MOUSE_MIDDLE == button) st->mouse.buttons.middle = true;
    st->flags.mouse_event = true;
}
void gui_event_mouseup(GuiState* st, int button) {
    if (MOUSE_LEFT   == button) st->mouse.buttons.left   = false;
    if (MOUSE_RIGHT  == button) st->mouse.buttons.right  = false;
    if (MOUSE_MIDDLE == button) st->mouse.buttons.middle = false;
    st->flags.mouse_event = true;
}
void gui_event_mousemove(GuiState* st, int x, int y) {
    st->mouse.x = x/st->scale;
    st->mouse.y = y/st->scale;
    st->flags.mouse_event = true;
}

void _gui_layout_push(GuiState* st, void* layout) {
    ((struct _GuiLayoutBase*)layout)->parent = st->layout;
    st->layout = (struct _GuiLayoutBase*)layout;
}
void _gui_layout_pop(GuiState* st, void* layout) {
    st->layout = ((struct _GuiLayoutBase*)layout)->parent;
}

void gui_layout_fixed_push(GuiState* st, GuiLayoutFixed* self) {
    _gui_layout_push(st, self);
    if (st->flags.need_redraw) {
        glColor4f(.42, .37, .40, .4);
        glRecti(self->rect.x, self->rect.y, self->rect.x+self->rect.width, self->rect.y+self->rect.height);
    }
}
void gui_layout_fixed_pop(GuiState* st, GuiLayoutFixed* self) {
    _gui_layout_pop(st, self);
}

void gui_layout_splits_push(GuiState* st, GuiLayoutSplits* self) {
    _gui_layout_push(st, self);
    self->current = 0;
}
void gui_layout_splits_next(GuiState* st, GuiLayoutSplits* self) {
    (void)st;
    //assert(self->current < self->count);
    //assert(st->layout == (void*)self);
    self->current++;
    float f = 1.f/self->count;
    switch (self->direction) {
        case SPLITS_VERTICAL:
            self->rect.width = self->parent->rect.width * f;
            self->rect.height = self->parent->rect.height;
            self->rect.x = self->parent->rect.x + self->parent->rect.width * (self->current-1)*f;
            self->rect.y = self->parent->rect.y;
            break;
        case SPLITS_HORIZONTAL:
            self->rect.width = self->parent->rect.width;
            self->rect.height = self->parent->rect.height * f;
            self->rect.x = self->parent->rect.x;
            self->rect.y = self->parent->rect.y + self->parent->rect.height * (self->current-1)*f;
            break;
        //default: false
    }
}
void gui_layout_splits_pop(GuiState* st, GuiLayoutSplits* self) {
    _gui_layout_pop(st, self);
}

static GuiRect _layout_rect_pos(GuiState* st, float ww, float hh, float padx, float pady) {
    GuiRect const l = st->layout->rect;
    GuiRect r;
    r.width = ww;
    r.height = hh;
    // this is for it centered, but we could take some params for hz/ve positioning
    r.x = l.x + l.width/2.f - r.width/2.f;
    r.y = l.y + l.height/2.f - r.height/2.f;
    return rect_pad(r, padx, pady);
}

void gui_label(GuiState* st, GuiLabel* self) {
    int ww, hh;
    text_area(self->text, strlen(self->text), &ww, &hh);

    static int const padx = 2;
    static int const pady = 2;
    GuiRect const r = _layout_rect_pos(st, ww, hh, padx, pady);

    if (st->flags.need_redraw) {
        glColor4f(1, .4, .7, 1);
        text_draw(self->text, strlen(self->text), r.x, r.y);
    }
}

void gui_button(GuiState* st, GuiButton* self) {
    int ww, hh;
    text_area(self->text, strlen(self->text), &ww, &hh);

    static int const padx = 2;
    static int const pady = 2;
    GuiRect const r = _layout_rect_pos(st, ww, hh, padx, pady);

    bool is_in = rect_in(r, st->mouse.x, st->mouse.y);
    bool is_down = st->flags.mouse_event && st->mouse.buttons.left;
    bool is_up = st->flags.mouse_event && !st->mouse.buttons.left;

    GuiButtonState const pstate = self->state;
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

    if (pstate != self->state) {
        st->flags.mouse_event = false; // XXX:/
        st->flags.will_need_redraw = true;
    }

    if (st->flags.need_redraw) {
        glColor4f(.42, .72, .12, 1);
        glRecti(r.x, r.y, r.x+r.width, r.y+r.height);

        switch (self->state) {
            case BUTTON_RESTING:  glColor4f(.88, .66, .77, 1); break;
            case BUTTON_HOVERED:  glColor4f(  1, .88, .99, 1); break;
            case BUTTON_PRESSED:  glColor4f(.66, .44, .55, 1); break;
            case BUTTON_RELEASED: glColor4f(.88, .66, .77, 1); break;
            case BUTTON_DISABLED: glColor4f(.44, .22, .33, 1); break;
        }

        text_draw(self->text, strlen(self->text), r.x+padx, r.y+pady);
    }
}

void gui_menu(GuiState* st, GuiMenu* self) {
    switch (self->state) {
        case MENU_RESTING:
            break;

        case MENU_OPENED:
            if (st->flags.mouse_event && st->mouse.buttons.left
                    && !rect_in(self->box.rect, st->mouse.x, st->mouse.y)) {
                st->flags.mouse_event = false;
                st->flags.will_need_redraw = true;
                self->state = MENU_RESTING;
                break;
            }

            gui_layout_fixed_push(st, &self->box);
            {
                self->splits.direction = SPLITS_HORIZONTAL;
                self->splits.count = self->count;

                gui_layout_splits_push(st, &self->splits);
                for (unsigned k = 0; k < self->count; k++) {
                    gui_layout_splits_next(st, &self->splits);
                    gui_button(st, &self->choices[k]);
                    if (BUTTON_RELEASED == self->choices[k].state) {
                        self->state = MENU_SELECTED;
                        self->pick = k;
                    }
                }
                gui_layout_splits_pop(st, &self->splits);
            }
            gui_layout_fixed_pop(st, &self->box);
            break;

        case MENU_SELECTED:
            self->state = MENU_RESTING;
            break;
    }
}

void gui_menu_alt(GuiState* st, GuiMenu* self) {
    switch (self->state) {
        case MENU_RESTING:
            if (st->flags.mouse_event && st->mouse.buttons.right) {
                st->flags.mouse_event = false;
                self->state = MENU_OPENED;
                if (!st->flags.need_redraw) st->flags.will_need_redraw = true;
                self->box.rect.x = st->mouse.x;
                self->box.rect.y = st->mouse.y;
                self->box.rect.width = 60;
                self->box.rect.height = self->count*16;
                self->box.rect = rect_pad(self->box.rect, 4, 4);
                if (!st->flags.need_redraw) return;
            }
            break;

        case MENU_OPENED:
            if (st->flags.mouse_event && st->mouse.buttons.right) {
                st->flags.mouse_event = false;
                if (!st->flags.need_redraw) st->flags.will_need_redraw = true;
                self->box.rect.x = st->mouse.x;
                self->box.rect.y = st->mouse.y;
                self->box.rect.width = 60;
                self->box.rect.height = self->count*16;
                self->box.rect = rect_pad(self->box.rect, 4, 4);
            }
            break;

        case MENU_SELECTED:
            break;
    }

    gui_menu(st, self);
}

#endif // GUI_IMPLEMENTATION
