#include <stdio.h>

#define FRAME_IMPLEMENTATION
#include "../ext/views/frame.h"
#define TEXT_IMPLEMENTATION
#include "../ext/views/text.h"
#define GUI_IMPLEMENTATION
#include "../ext/views/gui.h"

static GuiState gst = {.scale= 3};
void my_gui_logic(Frame* f) {
    gui_begin(&gst);
    {
        static GuiLayoutSplits lo = {.direction= SPLITS_HORIZONTAL, .count= 2};
        gui_layout_splits(&gst, &lo);

        gui_layout_push(&gst, &lo);
        {
            static char button_text[16] = {0};
            if (!*button_text) strcpy(button_text, "press me");
            static GuiButton my_button = {.text= button_text};
            gui_layout_splits_next(&gst, &lo);
                gui_button(&gst, &my_button);

            static GuiLabel my_label = {0};
            switch (my_button.state) {
                case BUTTON_RESTING:  my_label.text = "BUTTON_RESTING";  break;
                case BUTTON_HOVERED:  my_label.text = "BUTTON_HOVERED";  break;
                case BUTTON_PRESSED:  my_label.text = "BUTTON_PRESSED";  break;
                case BUTTON_RELEASED: my_label.text = "BUTTON_RELEASED"; break;
                case BUTTON_DISABLED: my_label.text = "BUTTON_DISABLED"; break;
            }
            gui_layout_splits_next(&gst, &lo);
                gui_label(&gst, &my_label);

            if (BUTTON_RELEASED == my_button.state) {
                static int press_count = 0;
                sprintf(button_text, "pressed %d", ++press_count);
                if (5 == press_count) {
                    my_button.state = BUTTON_DISABLED;
                    my_button.text = "no more pressing!";
                }
            }
        }
        gui_layout_pop(&gst, &lo);
    }
    gui_end(&gst);
    if (gui_need_redraw(&gst)) frame_redraw(f);
}

void render(Frame* f) {
    (void)f;
    my_gui_logic(f);
    static int counter = 0;
    printf("redrawn! %d times\n", ++counter);
}

void resize(Frame* f, int w, int h) {
    // NOTE: frame.h could be filtering the first few 'resize' events
    //       as from my testing these can be pretty wild..
    if (1 < w && 1 < h) {
        gui_event_reshape(&gst, w, h, gst.scale);
        my_gui_logic(f);
    }
}

void keydown(Frame* f, unsigned key) {
    (void)f;
    printf("keydown(0x%02X)\n", key);
    if (KEY_ESC == key) frame_close(f);
    if (KEY_SPACE == key) {
        gst.flags.need_redraw = true;
        frame_redraw(f);
    }
}

void mousedown(Frame* f, int button, int x, int y) {
    (void)x;
    (void)y;
    gui_event_mousedown(&gst, button);
    my_gui_logic(f);
}

void mouseup(Frame* f, int button, int x, int y) {
    (void)x;
    (void)y;
    gui_event_mouseup(&gst, button);
    my_gui_logic(f);
}

void mousewheel(Frame* f, int delta, int x, int y) {
    (void)x;
    (void)y;
    gui_event_reshape(&gst, f->width, f->height, gst.scale * (delta < 0 ? 0.9f : 1.1f));
    my_gui_logic(f);
}

void mousemove(Frame* f, int x, int y) {
    gui_event_mousemove(&gst, x, y);
    my_gui_logic(f);
}

int main(void) {
    Frame f = {
        .width= 640,
        .height= 480,
        .title= "hm",
        .events= {
            .render= render,
            .resize= resize,
            .closing= frame_close,
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
