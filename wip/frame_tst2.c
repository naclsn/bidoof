#include <stdio.h>
#include <string.h>

#define FRAME_IMPLEMENTATION
#include "../ext/views/frame.h"
#define TEXT_IMPLEMENTATION
#include "../ext/views/text.h"

void render(Frame* f) {
    (void)f;
    glClearColor(.12f, .13f, .14f, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    char const* const txt = "hi :>\r\nhow you?";
    text_draw(txt, strlen(txt), 0, f->height-8*5.f, 5.f);
}

void resize(Frame* f, int w, int h) {
    (void)f;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, w, 0.0, h, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
}

void keydown(Frame* f, char key) {
    if (27 == key || 9 == key) frame_close(f);
}

int main(int argc, char** argv) {
    argc--, argv++;

    Frame f = {
        .width= 640,
        .height= 480,
        .title= "hi :3",
        .events= {
            .render= render,
            .resize= resize,
            .closing= frame_close,
            .keydown= keydown,
        },
    };

    if (!frame_create(&f)) return 1;
    text_init();
    frame_loop(&f);
    text_free();
    frame_destroy(&f);

    return 0;
}
