#include <stdio.h>

#define FRAME_IMPLEMENTATION
#include "../ext/views/frame.h"
#define TEXT_IMPLEMENTATION
#include "../ext/views/text.h"
#define GUI_IMPLEMENTATION
#include "../ext/views/gui.h"

gui_state(gst);
void my_gui_logic(Frame* f) {
    gui_begin(&gst);

    gui_button(&gst, my_button, "press me", true);
    if (my_button.pressed) puts("button pressed!");
    if (my_button.released) puts("button released!");

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
