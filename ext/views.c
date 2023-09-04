// XXX: XXXXXXXXXXXXX: this works out of luck! at least on this Windows+Cygwin,
// changing anything in the `Frame` structure (which impacts its size) just
// doesn't work and passes garbage around
#include "../helper.h"

#define FRAME_IMPLEMENTATION
#include "views/frame.h"
#include <pthread.h>
#include <unistd.h> // sleep

export_names("ViewTxt");

ctor_w_also(1, ViewTxt, _ViewTxt_once
        , "view and edit buffer as text"
        , (1, BUF, _ViewTxt, BUF, txt)
        );

typedef struct ViewTxtState {
    Frame frame;
    pthread_t thread;
    u8 ptr[];
} ViewTxtState;

void _ViewTxt_render(Frame* frame) {
    pthread_testcancel();
    puts("redrawing!");
    ViewTxtState* st = frommember(frame, ViewTxtState, frame);
    (void)st;
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

// XXX: ZZZ
void myframe_destroy(Frame* self) {
    puts("destroy!!!");
    frame_destroy(self);
}

void* _ViewTxt_thread(Frame* frame) {
    pthread_cleanup_push((void(*)(void*))myframe_destroy, frame);
    if (frame_create(frame)) frame_loop(frame);
    pthread_cleanup_pop(true);
    return NULL;
}

bool _ViewTxt_once(Obj* fun, Obj* res) {
    (void)fun;

    ViewTxtState* st = calloc(1, sizeof *st);
    st->frame.width = 256;
    st->frame.height = 256;
    st->frame.title = "Some ViewTxt";
    st->frame.events.render = _ViewTxt_render;
    st->frame.events.closing = frame_close;

    res->as.buf.ptr = st->ptr;

    puts("== creating frame in background thread");

    // TODO: should also listen for any failure from `frame_create`
    return 0 == pthread_create(&st->thread, NULL, (void*(*)(void*))_ViewTxt_thread, &st->frame)
        || 0 == pthread_detach(st->thread);
}

bool _ViewTxt(Buf* self, Buf const* const txt) {
    ViewTxtState* st = frommember(self->ptr, ViewTxtState, ptr);

    if (destroyed(self)) {
        puts("== frame close request");
        if (0 == pthread_cancel(st->thread)) frame_redraw(&st->frame);
        puts("== request sent");
        sleep(1);
        puts("== done sleeping");
        free(st);
        return true;
    }

    if (self->len < txt->len) {
        ViewTxtState tmp = *st;

        free(st);
        st = malloc(sizeof *st + txt->len);
        if (!st) return false;
        memcpy(st, &tmp, sizeof *st);

        self->ptr = st->ptr;
    }

    memcpy(self->ptr, txt->ptr, self->len = txt->len);
    return true;
}
