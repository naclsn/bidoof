// on Windows it needed: -lopengl32 -lgdi32

#define FRAME_IMPLEMENTATION
#include "ext/views/frame.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void render_triangle(Frame* frame) {
    (void)frame;

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

    pthread_testcancel();
}

void close_on_escape(Frame* frame, char key) {
    printf("key released! 0x%02X\n", key);
    if (27 == key) frame_close(frame);
}

void* myframe_main(Frame* frame) {
    pthread_cleanup_push((void*)frame_destroy, frame);
        if (frame_create(frame, "window title"))
            frame_loop(frame);
    pthread_cleanup_pop(true);
    return NULL;
}

int main(void) {
    Frame frame = {
        .width= 256,
        .height= 256,
        .events= {
            .render= render_triangle,
            .closing= frame_close,
            .keyup= close_on_escape,
        },
    };

    pthread_t thr; {
        int r;
        if ((r = pthread_create(&thr, NULL, (void*)myframe_main, &frame))) return r;
        if ((r = pthread_detach(thr))) return r;
    }

    for (int k = 0; k < 5; k++) {
        sleep(1);
        printf("%ds\n", k+1);
    }

    if (0 == pthread_cancel(thr))
        frame_redraw(&frame); // force a cancelation point (win32 event loop isn't one :/)

    sleep(1); // need to give time to reach the cancelation point
    return 0;
}
