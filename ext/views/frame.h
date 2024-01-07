/// See the end of the file for an example; the idea is as follow:
/// - create a `Frame` structure with a size, title and events
/// - call `frame_create` (it returns false on failure)
/// - then `frame_loop` to make it live, this is blocking
/// - finally `frame_destroy` when it falls out of it
///
/// Calling `frame_redraw` forces a render event.
/// Calling `frame_close` makes it fall out of the loop.
/// I have no clue as for the thread-safety of anything.
///
/// Make sure to only use FRAME_IMPLEMENTATION once.
/// (TODO: make it possible to select the impl with the macro.)
/// If any of _WIN32, _WIN64 or __CYGWIN__ is present, uses Win32 API.
/// Otherwise, only X11 is supported for now.

#include <stdbool.h>

typedef struct Frame Frame;

bool frame_create(Frame* self);
void frame_loop(Frame* self);
void frame_redraw(Frame* self);
void frame_close(Frame* self);
void frame_destroy(Frame* self);

unsigned frame_key2char(unsigned key);

#define extends_FrameBase                                            \
    int width;                                                       \
    int height;                                                      \
    char const* title;                                               \
                                                                     \
    struct {                                                         \
        void (*render)(Frame* self);                                 \
        void (*resize)(Frame* self, int w, int h);                   \
        void (*closing)(Frame* self);                                \
                                                                     \
        void (*keydown)(Frame* self, unsigned key);                  \
        void (*keyup)(Frame* self, unsigned key);                    \
                                                                     \
        void (*mousedown)(Frame* self, int button, int x, int y);    \
        void (*mouseup)(Frame* self, int button, int x, int y);      \
                                                                     \
        void (*mousewheel)(Frame* self, int delta, int x, int y);    \
        void (*mousemove)(Frame* self, int x, int y);                \
    } events;                                                        \

#define _event_frame(__frame, ...) __frame
#define _event(__name, ...) do                            \
    if (_event_frame(__VA_ARGS__,)->events.__name)               \
        _event_frame(__VA_ARGS__,)->events.__name(__VA_ARGS__);  \
    while (false)

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#include "_win32.h"
#else
#include "_x11.h"
#endif

#undef _event
#undef extends_FrameBase

#if 0
#define FRAME_IMPLEMENTATION
#include "(this file)"
#include <pthread.h> // if using main_threaded
#include <unistd.h> // sleep

void triangle(Frame* frame) {
    pthread_testcancel(); // if using main_threaded
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
Frame frame = {
    .width= 256,
    .height= 256,
    .title= "wow! window! crazy!",
    .events= {
        .render= triangle,
        .closing= frame_close,
    },
};

void* myframe_main(Frame* frame) {
    pthread_cleanup_push((void*)frame_destroy, frame);
        if (frame_create(frame)) frame_loop(frame);
    pthread_cleanup_pop(true);
    return NULL;
}
int main_threaded(void) {
    pthread_t thr;
    int r;
    if ((r = pthread_create(&thr, NULL, (void*)myframe_main, &frame)) || (r = pthread_detach(thr))) return r;
    sleep(5); // sleep a bit in the mean time
    if (!pthread_cancel(thr)) frame_redraw(&frame); // force a cancelation point (win32 event loop isn't one :/)
    sleep(1); // give time to reach the cancelation point
    return 0;
}

int main_nothread(void) {
    if (!frame_create(&frame)) return 1;
    frame_loop(&frame);
    frame_destroy(&frame);
    return 0;
}
#endif
