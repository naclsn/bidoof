#include <stdio.h>

#define FRAME_IMPLEMENTATION
#include "../ext/views/frame.h"
#define TEXT_IMPLEMENTATION
#include "../ext/views/text.h"
#define GUI_IMPLEMENTATION
#include "../ext/views/gui.h"

gui_state(gst);

void render(Frame* f) {
    (void)f;
    glClearColor(.12f, .15f, .18f, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    //glColor4f(.88f, .66f, .77f, 1.f);
    //char const* const txt = "hi and such\r\n\u3057\u304A";
    //text_draw(txt, strlen(txt), 2, 10, 10);

    gui_button(my_button, &gst, "press me", true);
    if (my_button.pressed) {
        puts("button pressed!");
    }

    puts("redrawn!");
}

void resize(Frame* f, int w, int h) {
    gui_event_reshape(&gst, w, h, gst.scale);
    frame_redraw(f);
}

void keydown(Frame* f, unsigned key) {
    (void)f;
    printf("keydown(0x%02X)\n", key);
    if (KEY_ESC == key) frame_close(f);
}

void mousedown(Frame* f, int button, int x, int y) {
    (void)f;
    printf("mousedown(%d, %d, %d)\n", button, x, y);
}

void mousewheel(Frame* f, int delta, int x, int y) {
    (void)x;
    (void)y;
    gui_event_reshape(&gst, gst.width, gst.height, gst.scale * (delta < 0 ? 0.9f : 1.1f));
    frame_redraw(f);
}

void mousemove(Frame* f, int x, int y) {
    gui_event_mousemove(&gst, x, y);
    frame_redraw(f);
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
