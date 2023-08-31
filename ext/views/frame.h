#include <stdbool.h>

typedef struct Frame Frame;

bool frame_create(Frame* self, char const* title);
void frame_loop(Frame* self);
void frame_close(Frame* self);
void frame_destroy(Frame* self);

#define _event(__f, __evn) if (__f->events.__evn) __f->events.__evn

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#include "_win32.h"
#else
#include "_x11.h"
#endif

#undef _event

struct Frame {
    int width;
    int height;

    struct {
        void (*render)(Frame* self);
        void (*resize)(Frame* self, int w, int h);
        void (*closing)(Frame* self);

        void (*keydown)(Frame* self, char key);
        void (*keyup)(Frame* self, char key);
        void (*keychar)(Frame* self, char key); // YYY: maybe not

        void (*mousedown)(Frame* self, int button, int x, int y);
        void (*mouseup)(Frame* self, int button, int x, int y);
        void (*mousedouble)(Frame* self, int button, int x, int y); // YYY: maybe not

        void (*mousewheel)(Frame* self, int delta, int x, int y);
        void (*mousemove)(Frame* self, int x, int y);
    } events;

    struct FrameImpl _impl;
};
