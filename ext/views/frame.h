#include <stdbool.h>

typedef struct Frame Frame;

bool frame_create(Frame* self, char const* title);
void frame_loop(Frame* self);
void frame_redraw(Frame* self);
void frame_close(Frame* self);
void frame_destroy(Frame* self);

#define extends_FrameBase struct {                                   \
    int width;                                                       \
    int height;                                                      \
                                                                     \
    struct {                                                         \
        void (*render)(Frame* self);                                 \
        void (*resize)(Frame* self, int w, int h);                   \
        void (*closing)(Frame* self);                                \
                                                                     \
        void (*keydown)(Frame* self, char key);                      \
        void (*keyup)(Frame* self, char key);                        \
        void (*keychar)(Frame* self, char key);                      \
                                                                     \
        void (*mousedown)(Frame* self, int button, int x, int y);    \
        void (*mouseup)(Frame* self, int button, int x, int y);      \
        void (*mousedouble)(Frame* self, int button, int x, int y);  \
                                                                     \
        void (*mousewheel)(Frame* self, int delta, int x, int y);    \
        void (*mousemove)(Frame* self, int x, int y);                \
    } events;                                                        \
}

#define _HEAD(__frame, ...) __frame
#define _event(__name, ...) do                    \
    if (_HEAD(__VA_ARGS__,)->events.__name)               \
        _HEAD(__VA_ARGS__,)->events.__name(__VA_ARGS__);  \
    while (false)

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#include "_win32.h"
#else
#include "_x11.h"
#endif

#undef _event
#undef extends_FrameBase
