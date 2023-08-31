// on Windows it needed: -lopengl32 -lgdi32

#include "ext/views/frame.h"
#include <stdio.h>

void render_triangle(Frame* self) {
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex2i(0, 1);
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex2i(-1, -1);
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex2i(1, -1);
    glEnd();
    glFlush();
}

void confirm_closing(Frame* self) {
    if (IDOK == MessageBox(self->hWnd, "close?", "hey", MB_OKCANCEL))
        frame_close(self);
}

void close_on_escape(Frame* self, char key) {
    printf("key pressed! 0x%02X\n", key);
    if (27 == key) frame_close(self);
}

int main(void) {
    Frame frame = {
        .width= 256,
        .height= 256,
        .events= {
            .render= render_triangle,
            .closing= confirm_closing,
            .keychar= close_on_escape,
        },
    };
    if (!frame_create(&frame, "window title")) return 1;
    frame_loop(&frame);
    frame_destroy(&frame);
    puts("clean exit");
    return 0;
}
