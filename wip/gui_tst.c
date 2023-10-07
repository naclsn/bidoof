#include <stdio.h>

#define FRAME_IMPLEMENTATION
#include "../ext/views/frame.h"
#define TEXT_IMPLEMENTATION
#include "../ext/views/text.h"
#define GUI_IMPLEMENTATION
#include "../ext/views/gui.h"

static GuiState gst = {.scale= 2.4};
void my_gui_logic(Frame* f) {
    gui_begin(&gst);
    {
        static GuiLayoutSplits lo = {.direction= SPLITS_HORIZONTAL, .count= 2};
        gui_layout_splits_push(&gst, &lo);
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
        gui_layout_splits_pop(&gst, &lo);

        static GuiMenu alt = {.count= 3, .choices= {
            {.text= "hello"},
            {.text= "bonjour"},
            {.text= "\xe3\x81\x93\xe3\x82\x93\xe3\x81\xab\xe3\x81\xa1\xe3\x82\x8f"},
        }};
        gui_menu_alt(&gst, &alt);
        if (MENU_SELECTED == alt.state) {
            static char* const ans[] = {
                "well hi there",
                "bien le bonjour",
                "\xe3\x81\xb8\xe3\x81\x84\xe3\x81\x8d\xe3\x81\xaa\xe3\x81\xae",
            };
            printf("%s\n", ans[alt.pick]);
        }
    }
    gui_end(&gst);

    /*switch (gst.state) {
        case GUI_RESTING: printf("GUI_RESTING"); break;
        case GUI_REDRAWING: printf("GUI_REDRAWING"); break;
        case GUI_MOUSE_EVENT_BUBBLE: printf("GUI_MOUSE_EVENT_BUBBLE"); break;
        case GUI_MOUSE_EVENT_CAPTURED: printf("GUI_MOUSE_EVENT_CAPTURED by %p", gst.event_captured); break;
        case GUI_KEY_EVENT_BUBBLE: printf("GUI_KEY_EVENT_BUBBLE"); break;
        case GUI_KEY_EVENT_CAPTURED: printf("GUI_KEY_EVENT_CAPTURED by %p", gst.event_captured); break;
        default: printf("broken state"); break;
    }
    printf("\n");*/

    if (gui_needed_redraw(&gst)) frame_redraw(f);
    else if (gui_needed_reloop(&gst)) my_gui_logic(f);
}

void render(Frame* f) {
    (void)f;
    my_gui_logic(f);
    static int counter = 0;
    printf("redrawn! %d times\n", ++counter);
}

void resize(Frame* f, int w, int h) {
    // TODO: frame.h could be filtering the first few 'resize' events
    //       as these can be pretty wild.. (seems to be an X11 thing)
    if (1 < w && 1 < h) {
        gui_event_reshape(&gst, w, h, gst.scale);
        frame_redraw(f); // YYY: special case
    }
}

void keydown(Frame* f, unsigned key) {
    (void)f;
    printf("keydown(0x%02X)\n", key);
    if (KEY_ESC == key) frame_close(f);
    if (KEY_SPACE == key) {
        gst.state = GUI_REDRAWING; // ZZZ: maybe user code shouldn't do that too much
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
