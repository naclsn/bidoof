#include <stdio.h>

#define FRAME_IMPLEMENTATION
#include "../ext/views/frame.h"
#define TEXT_IMPLEMENTATION
#include "../ext/views/text.h"
#define GUI_IMPLEMENTATION
#include "../ext/views/gui.h"

// TODO: typedefs/macros for public api types
static struct _GuiState gst = {0};
void my_gui_logic(Frame* f) {
    gui_begin(&gst);

    static char button_text[16] = {0};
    if (!*button_text) strcpy(button_text, "press me");
    static struct _GuiButton my_button = {.text= button_text};
    gui_button(&gst, &my_button);

    static char const* curr_text;
    switch (my_button.state) {
        case BUTTON_RESTING:  curr_text = "BUTTON_RESTING";  break;
        case BUTTON_HOVERED:  curr_text = "BUTTON_HOVERED";  break;
        case BUTTON_PRESSED:  curr_text = "BUTTON_PRESSED";  break;
        case BUTTON_RELEASED: curr_text = "BUTTON_RELEASED"; break;
        case BUTTON_DISABLED: curr_text = "BUTTON_DISABLED"; break;
    }
    gui_text(&gst, curr_text);

    // FIXME: gets incremented twice
    if (BUTTON_RELEASED == my_button.state) {
        static int press_count = 0;
        sprintf(button_text, "pressed %d", ++press_count);
        if (5 == press_count) my_button.state = BUTTON_DISABLED;
    }

    gui_end(&gst);
    if (gui_need_redraw(&gst)) frame_redraw(f);
}

void render(Frame* f) {
    (void)f;
    my_gui_logic(f);
    puts("redrawn!");
}

void resize(Frame* f, int w, int h) {
    gui_event_reshape(&gst, w, h, gst.scale);
    my_gui_logic(f);
}

void keydown(Frame* f, unsigned key) {
    (void)f;
    printf("keydown(0x%02X)\n", key);
    if (KEY_ESC == key) frame_close(f);
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
    gui_event_reshape(&gst, gst.width, gst.height, gst.scale * (delta < 0 ? 0.9f : 1.1f));
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
