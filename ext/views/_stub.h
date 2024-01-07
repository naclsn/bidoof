#include <GL/gl.h>

struct Frame { extends_FrameBase
};

#ifdef FRAME_IMPLEMENTATION
bool frame_create(Frame* self) { (void)self; return false; }
void frame_loop(Frame* self) { (void)self; }
void frame_redraw(Frame* self) { (void)self; }
void frame_close(Frame* self) { (void)self; }
void frame_destroy(Frame* self) { (void)self; }
#endif // FRAME_IMPLEMENTATION
