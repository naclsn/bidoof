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

    enum {
        GUI_RESTING,
        GUI_REDRAWING,
        GUI_MOUSE_EVENT_BUBBLE,
        GUI_MOUSE_EVENT_CAPTURED,
        GUI_KEY_EVENT_BUBBLE,
        GUI_KEY_EVENT_CAPTURED,
        //GUI_OTHER_EVENT,
    } state;

    void* event_captured;
    bool needs_redraw;

    struct {
        float x, y;
        enum {
            GUI_MOUSE_DOWN_LEFT = MOUSE_LEFT,
            GUI_MOUSE_DOWN_RIGHT = MOUSE_RIGHT,
            GUI_MOUSE_DOWN_MIDDLE = MOUSE_MIDDLE,
            GUI_MOUSE_UP_LEFT = MOUSE_LEFT+3,
            GUI_MOUSE_UP_RIGHT = MOUSE_RIGHT+3,
            GUI_MOUSE_UP_MIDDLE = MOUSE_MIDDLE+3,
        } button;
    } mouse;
} GuiState;

void gui_begin(GuiState* st);
void gui_end(GuiState* st);
#define gui_needed_redraw(__st)  (GUI_REDRAWING == (__st)->state)
#define gui_needed_reloop(__st)  (GUI_RESTING != (__st)->state)

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
    if (GUI_REDRAWING == st->state) {
        glClearColor(.12f, .15f, .18f, 1);
        glClear(GL_COLOR_BUFFER_BIT);
    }
}
void gui_end(GuiState* st) {
    switch (st->state) {
        case GUI_RESTING: break;
        case GUI_REDRAWING: st->state = GUI_RESTING; break;

        case GUI_MOUSE_EVENT_BUBBLE: st->state = st->event_captured ? GUI_MOUSE_EVENT_CAPTURED : GUI_RESTING; break;
        case GUI_KEY_EVENT_BUBBLE: st->state = st->event_captured ? GUI_KEY_EVENT_CAPTURED : GUI_RESTING; break;

        case GUI_MOUSE_EVENT_CAPTURED:
        case GUI_KEY_EVENT_CAPTURED:
            st->event_captured = NULL;
            st->state = GUI_RESTING;
            break;
    }

    if (st->needs_redraw && GUI_RESTING == st->state)
        st->state = GUI_REDRAWING;
    st->needs_redraw = false;
}

void gui_event_reshape(GuiState* st, int w, int h, float scale) {
    st->rect.width = w/scale;
    st->rect.height = h/scale;
    st->scale = scale;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, w/scale, h/scale, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    st->state = GUI_REDRAWING;
}

void gui_event_mousedown(GuiState* st, int button) {
    st->mouse.button = button;
    st->state = GUI_MOUSE_EVENT_BUBBLE;
}
void gui_event_mouseup(GuiState* st, int button) {
    st->mouse.button = button+3;
    st->state = GUI_MOUSE_EVENT_BUBBLE;
}
void gui_event_mousemove(GuiState* st, int x, int y) {
    st->mouse.x = x/st->scale;
    st->mouse.y = y/st->scale;
    //st->state = GUI_MOUSE_EVENT_BUBBLE;
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
    if (GUI_REDRAWING == st->state) {
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

    if (GUI_REDRAWING == st->state) {
        glColor4f(1, .4, .7, 1);
        text_draw(self->text, strlen(self->text), r.x, r.y);
    }
}

void gui_button(GuiState* st, GuiButton* self) {
    // TODO: focused return/space
    if (GUI_KEY_EVENT_BUBBLE == st->state) return; // YYY: not interested yet

    int ww, hh;
    text_area(self->text, strlen(self->text), &ww, &hh);

    static int const padx = 2;
    static int const pady = 2;
    GuiRect const r = _layout_rect_pos(st, ww, hh, padx, pady);

    bool is_in = rect_in(r, st->mouse.x, st->mouse.y);
    if (GUI_MOUSE_EVENT_BUBBLE == st->state) {
        if (is_in && (GUI_MOUSE_DOWN_LEFT == st->mouse.button
                   || GUI_MOUSE_UP_LEFT == st->mouse.button
                )) st->event_captured = self;
        return;
    }

    bool captured = GUI_MOUSE_EVENT_CAPTURED == st->state && self == st->event_captured;
    bool is_down = captured && GUI_MOUSE_DOWN_LEFT == st->mouse.button;
    bool is_up = captured && GUI_MOUSE_UP_LEFT == st->mouse.button;

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

    if (pstate != self->state)
        st->needs_redraw = true;

    if (GUI_REDRAWING == st->state) {
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
    if (GUI_KEY_EVENT_BUBBLE == st->state) return; // YYY: not interested yet

    switch (self->state) {
        case MENU_RESTING:
            break;

        case MENU_OPENED:
            bool is_in = rect_in(self->box.rect, st->mouse.x, st->mouse.y);
            if (GUI_MOUSE_EVENT_BUBBLE == st->state) {
                if (!is_in) {
                    // well in this case we know it's not for us
                    st->needs_redraw = true;
                    self->state = MENU_RESTING;
                    return;
                }

                // capture just in case (prevent click-through)
                if (is_in) st->event_captured = self;
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
            if (GUI_MOUSE_EVENT_BUBBLE == st->state) return;
            st->needs_redraw = true;
            self->state = MENU_RESTING;
            break;
    }
}

void gui_menu_alt(GuiState* st, GuiMenu* self) {
    // TODO: menu key?
    if (GUI_KEY_EVENT_BUBBLE == st->state) return; // YYY: not interested yet

    if (GUI_MOUSE_EVENT_BUBBLE == st->state) {
        // we don't take too much prioriy
        if (GUI_MOUSE_DOWN_RIGHT == st->mouse.button && !st->event_captured)
            st->event_captured = self;
        else
            gui_menu(st, self);
        return;
    }

    bool captured = GUI_MOUSE_EVENT_CAPTURED == st->state && self == st->event_captured;
    bool is_down = captured && GUI_MOUSE_DOWN_RIGHT == st->mouse.button;

    switch (self->state) {
        case MENU_RESTING:
        case MENU_OPENED:
            if (is_down) {
                st->needs_redraw = true;
                self->state = MENU_OPENED;
                self->box.rect.x = st->mouse.x;
                self->box.rect.y = st->mouse.y;
                self->box.rect.width = 60;
                self->box.rect.height = self->count*16;
                self->box.rect = rect_pad(self->box.rect, 4, 4);
                return;
            }
            break;

        case MENU_SELECTED:
            break;
    }

    gui_menu(st, self);
}

#endif // GUI_IMPLEMENTATION
