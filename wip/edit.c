#include <stdio.h>

#define FRAME_IMPLEMENTATION
#include "../ext/views/frame.h"
#define TEXT_IMPLEMENTATION
#include "../ext/views/text.h"
#define GUI_IMPLEMENTATION
#include "../ext/views/gui.h"

#include "../dyarr.h"

typedef struct uGuiEditor {
    dyarr(dyarr(char)) lines;
    size_t top;
    struct { size_t ln, col; } cur;
} uGuiEditor;
void ugui_editor_set(GuiState* st, uGuiEditor* self, char const* text);
char* ugui_editor_get(GuiState* st, uGuiEditor* self);
void _ugui_editor_key(GuiState* st, uGuiEditor* self, unsigned key);
void ugui_editor(GuiState* st, uGuiEditor* self);

static GuiState gst = {.scale= 2.4};
static uGuiEditor ged = {0};
void my_gui_logic(Frame* f) {
    gui_begin(&gst);
    {
        static bool first = true;
        if (first) {
            ugui_editor_set(&gst, &ged, "this is some text\nmade of lines\nand characters\n");
            first = false;
        }

        ugui_editor(&gst, &ged);

        static GuiMenu alt = {
            .count= 1,
            .choices= {
                {.text= "Save"}
            }
        };
        gui_menu_alt(&gst, &alt);
        if (MENU_SELECTED == alt.state) {
            switch (alt.pick) {
                case 0: { // "Save"
                    char* txt = ugui_editor_get(&gst, &ged);
                    printf("txt:\n```\n%s```\n\n", txt);
                    free(txt);
                } break;
            }
        } // alt.state
    }
    gui_end(&gst);

    if (gui_needed_redraw(&gst)) frame_redraw(f);
    else if (gui_needed_reloop(&gst)) my_gui_logic(f);
}

void render(Frame* f) {
    (void)f;
    my_gui_logic(f);
    static int counter = 0;
    printf("redrawn! %d times\n", ++counter);
}

void resize(Frame* f, int w, int h) { gui_event_reshape(&gst, w, h, gst.scale); frame_redraw(f); }

void mousedown(Frame* f, int button, int x, int y) { (void)x; (void)y; gui_event_mousedown(&gst, button); my_gui_logic(f); }
void mouseup(Frame* f, int button, int x, int y) { (void)x; (void)y; gui_event_mouseup(&gst, button); my_gui_logic(f); }
void mousewheel(Frame* f, int delta, int x, int y) { (void)x; (void)y; gui_event_reshape(&gst, f->width, f->height, gst.scale * (delta < 0 ? 0.9f : 1.1f)); my_gui_logic(f); }
void mousemove(Frame* f, int x, int y) { gui_event_mousemove(&gst, x, y); my_gui_logic(f); }

void keydown(Frame* f, unsigned key) {
    if (KEY_ESC == key) frame_close(f);
    //st->state = GUI_KEY_EVENT_BUBBLE;
    //my_gui_logic(f);

    //_ugui_editor_key(&ged, key);
    //gst.state = GUI_REDRAWING;
    //frame_redraw(f);

    //_ugui_editor_key(&gst, &ged, key);
    //my_gui_logic(f);

    gui_event_keydown(&gst, key);
    my_gui_logic(f);
}

void keyup(Frame* f, unsigned key) {
    gui_event_keyup(&gst, key);
    my_gui_logic(f);
}

int main(void) {
    Frame f = {
        .width= 640,
        .height= 480,
        .title= "hm",
        .events= {
            .closing= frame_close,
            .render= render,
            .resize= resize,
            .mousedown= mousedown,
            .mouseup= mouseup,
            .mousemove= mousemove,
            .mousewheel= mousewheel,
            .keydown= keydown,
            .keyup= keyup,
        },
    };

    if (!frame_create(&f)) return 1;
    text_init();
    frame_loop(&f);
    text_free();
    frame_destroy(&f);

    return 0;
}



void ugui_editor_set(GuiState* st, uGuiEditor* self, char const* text) {
    (void)st;
    bool nl = true;
    while (*text) {
        char c = *text++;
        if ('\n' == c) nl = true;
        else {
            if (nl) {
                void* niw = dyarr_push(&self->lines);
                if (!niw) return;
                memset(niw, 0, sizeof(dyarr(char)));
                nl = false;
            }
            char* cc = dyarr_push(&self->lines.ptr[self->lines.len-1]);
            if (!cc) return;
            *cc = c;
        }
    } // while text
}

char* ugui_editor_get(GuiState* st, uGuiEditor* self) {
    (void)st;
    dyarr(char) r = {0};
    for (size_t i = 0; i < self->lines.len; i++)
        for (size_t j = 0; j < self->lines.ptr[i].len + 1; j++) {
            char* cc = dyarr_push(&r);
            if (!cc) return r.ptr;
            *cc = j == self->lines.ptr[i].len ? '\n' : self->lines.ptr[i].ptr[j];
        }
    return r.ptr;
}

void _ugui_editor_key(GuiState* st, uGuiEditor* self, unsigned key) {
    printf("- captured event! key%s: %u, mod:%s%s -\n",
            st->keyboard.down ? "down" : "up",
            st->keyboard.key,
            st->keyboard.mod.shift ? " shift" : "",
            st->keyboard.mod.ctrl ? " ctrl" : "");

    switch (key) {
        case KEY_RETURN: {
                void* niw = dyarr_insert(&self->lines, self->cur.ln+1, 1);
                if (!niw) return;
                memset(niw, 0, sizeof(dyarr(char)));
                self->cur.ln++;
                size_t ends_len = self->lines.ptr[self->cur.ln-1].len - self->cur.col;
                void* ends_niw = dyarr_insert(&self->lines.ptr[self->cur.ln], 0, ends_len);
                if (!ends_niw) break;
                memcpy(ends_niw, self->lines.ptr[self->cur.ln-1].ptr + self->cur.col, ends_len);
                self->lines.ptr[self->cur.ln-1].len-= ends_len;
                self->cur.col = 0;
            } break;

        case KEY_BACKSPACE:
            if (self->cur.col) {
                self->cur.col--;
                dyarr_remove(&self->lines.ptr[self->cur.ln], self->cur.col, 1);
            } else if (!self->lines.ptr[self->cur.ln].len && 1 < self->lines.len) {
                dyarr_remove(&self->lines, self->cur.ln, 1);
                if (self->cur.ln) --self->cur.ln;
                self->cur.col = self->lines.ptr[self->cur.ln].len;
            }
            break;
        case KEY_DELETE:
            if (self->cur.col < self->lines.ptr[self->cur.ln].len)
                dyarr_remove(&self->lines.ptr[self->cur.ln], self->cur.col, 1);
            else if (!self->lines.ptr[self->cur.ln].len && 1 < self->lines.len) {
                dyarr_remove(&self->lines, self->cur.ln, 1);
                if (self->lines.len == self->cur.ln) --self->cur.ln;
            }
            break;

        case KEY_LEFT: if (self->cur.col) --self->cur.col; break;
        case KEY_RIGHT: if (self->cur.col < self->lines.ptr[self->cur.ln].len) ++self->cur.col; break;
        case KEY_UP: if (self->cur.ln && self->lines.ptr[--self->cur.ln].len < self->cur.col) self->cur.col = self->lines.ptr[self->cur.ln].len; break;
        case KEY_DOWN: if (self->cur.ln < self->lines.len-1 && self->lines.ptr[++self->cur.ln].len < self->cur.col) self->cur.col = self->lines.ptr[self->cur.ln].len; break;
        case KEY_HOME: self->cur.col = 0; break;
        case KEY_END: self->cur.col = self->lines.ptr[self->cur.ln].len; break;

        case KEY_PAGEUP: if (self->top) --self->top; break;
        case KEY_PAGEDOWN: if (self->top < self->lines.len-1) ++self->top; break;

        default:
            if (' ' <= key && key <= '~') {
                char* cc = dyarr_insert(&self->lines.ptr[self->cur.ln], self->cur.col, 1);
                if (!cc) return;
                *cc = key | 0b100000;
                self->cur.col++;
            }
    }

    st->needs_redraw = true;
}

void ugui_editor(GuiState* st, uGuiEditor* self) {
    if (GUI_REDRAWING == st->state) {
        GuiRect const r = rect_pad(st->layout->rect, -8, -8);
        glColor3f(.44, .44, .44);
        glRecti(r.x, r.y, r.x+r.width, r.y+r.height);

        glEnable(GL_SCISSOR_TEST);
        //glScissor(r.x, r.y, r.width, r.height);
        {
            GLint xx = r.x*st->scale;
            GLint yy = (st->rect.height-r.y-r.height)*st->scale;
            GLsizei ww = r.width*st->scale;
            GLsizei hh = r.height*st->scale;
            glScissor(xx, yy, ww, hh);
        }

        glColor3f(.88, .88, .88);
        for (size_t k = 0; k < self->lines.len && 9*k < r.height; k++)
            text_draw(self->lines.ptr[self->top+k].ptr, self->lines.ptr[self->top+k].len, r.x, r.y + 9*k);

        if (self->top <= self->cur.ln) {
            glColor3f(.72, .88, 0);
            GLint xx = r.x + 8*self->cur.col;
            GLint yy = r.y + 9*(self->cur.ln-self->top);
            glRecti(xx-1, yy-1, xx+2, yy+9);
        }

        glDisable(GL_SCISSOR_TEST);
    }

    else if (GUI_KEY_EVENT_BUBBLE == st->state)
        st->event_captured = self;
    else if (GUI_KEY_EVENT_CAPTURED == st->state && self == st->event_captured && st->keyboard.down)
        _ugui_editor_key(st, self, st->keyboard.key);
}
