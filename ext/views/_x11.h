//

struct Frame { extends_FrameBase;
    int a;
};

#ifdef FRAME_IMPLEMENTATION
#include <GL/gl.h>

bool frame_create(Frame* self, char const* title) {
    (void)self;
    (void)title;
    return false;
}

void frame_loop(Frame* self) {
    (void)self;
}

void frame_redraw(Frame* self) {
    (void)self;
}

void frame_close(Frame* self) {
    (void)self;
}

void frame_destroy(Frame* self) {
    (void)self;
}

#endif // FRAME_IMPLEMENTATION
