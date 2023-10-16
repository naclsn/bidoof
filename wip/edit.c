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
} uGuiEditor;
void ugui_editor_set(GuiState* st, uGuiEditor* self, char const* text);
char* ugui_editor_get(GuiState* st, uGuiEditor* self);
void _ugui_editor_key(uGuiEditor* self, unsigned key);
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

void keydown(Frame* f, unsigned key) {
    if (KEY_ESC == key) frame_close(f);
    //st->state = GUI_KEY_EVENT_BUBBLE;
    //my_gui_logic(f);

    _ugui_editor_key(&ged, key);
    gst.state = GUI_REDRAWING;
    frame_redraw(f);
}

void mousedown(Frame* f, int button, int x, int y) { (void)x; (void)y; gui_event_mousedown(&gst, button); my_gui_logic(f); }
void mouseup(Frame* f, int button, int x, int y) { (void)x; (void)y; gui_event_mouseup(&gst, button); my_gui_logic(f); }
void mousewheel(Frame* f, int delta, int x, int y) { (void)x; (void)y; gui_event_reshape(&gst, f->width, f->height, gst.scale * (delta < 0 ? 0.9f : 1.1f)); my_gui_logic(f); }
void mousemove(Frame* f, int x, int y) { gui_event_mousemove(&gst, x, y); my_gui_logic(f); }

int main(void) {
    Frame f = {
        .width= 640,
        .height= 480,
        .title= "hm",
        .events= {
            .closing= frame_close,
            .render= render,
            .resize= resize,
            .keydown= keydown,
            .mousedown= mousedown,
            .mouseup= mouseup,
            .mousewheel= mousewheel,
            .mousemove= mousemove,
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

void _ugui_editor_key(uGuiEditor* self, unsigned key) {
    if (KEY_RETURN == key) {
        void* niw = dyarr_push(&self->lines);
        if (!niw) return;
        memset(niw, 0, sizeof(dyarr(char)));
    } else if (KEY_BACKSPACE == key) {
        if (!dyarr_pop(&self->lines.ptr[self->lines.len-1]) && 1 < self->lines.len)
            dyarr_pop(&self->lines);
    } else if (' ' <= key && key <= '~') {
        char* cc = dyarr_push(&self->lines.ptr[self->lines.len-1]);
        if (!cc) return;
        *cc = key | 0b100000;
    }
}

void ugui_editor(GuiState* st, uGuiEditor* self) {
    (void)st;
    glColor3f(.88, .88, .88);
    for (size_t k = 0; k < self->lines.len; k++)
        text_draw(self->lines.ptr[k].ptr, self->lines.ptr[k].len, 8, 8+9*k);
}
