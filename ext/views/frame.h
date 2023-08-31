#include <stdbool.h>

typedef struct Frame Frame;

typedef struct FrameBase {
    int width;
    int height;

    struct FrameEventHandlers {
        void (*render)(Frame* self);
        void (*resize)(Frame* self, int w, int h);
        void (*closing)(Frame* self);
        void (*key_pressed)(Frame* self, char key);
        void (*mouse_moved)(Frame* self, int x, int y);
        void (*mouse_pressed)(Frame* self, int button);
    } events;
} FrameBase;

bool frame_create(Frame* self, char const* title);
void frame_loop(Frame* self);
void frame_close(Frame* self);
void frame_destroy(Frame* self);

#define _event(__f, __evn) if (__f->events.__evn) __f->events.__evn

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
# include "_win.h"
#else
# include "_linux.h"
#endif
