#define FRAME_IMPLEMENTATION
#include "views/frame.h"
#include <pthread.h>
#include <unistd.h> // ZZZ: sleep

#include "../helper.h"

export_names("ViewTxt");

static bool _frame_ok_st;
static bool _frame_ok_ok;
static pthread_mutex_t _frame_ok_mx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t _frame_ok_cv = PTHREAD_COND_INITIALIZER;

ctor_w_also(1, ViewTxt, _ViewTxt_once
        , "view and edit buffer as text"
        , (1, BUF, _ViewTxt, BUF, txt)
        );

typedef struct ViewTxtState {
    Frame frame;
    pthread_t thread;
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
        _frame_ok_st = frame_create(frame);
        if (_frame_ok_st) {
            _frame_ok_ok = true;
            pthread_cond_signal(&_frame_ok_cv);
            frame_loop(frame);
        } else goto failed;
    pthread_cleanup_pop(true);
    return NULL;

failed:
    _frame_ok_ok = true;
    pthread_cond_signal(&_frame_ok_cv);
    return NULL;
}

bool _ViewTxt_once(Obj* fun, Obj* res) {
    (void)fun;

    ViewTxtState* st = calloc(1, sizeof *st);
    st->frame.width = 256;
    st->frame.height = 256;
    st->frame.title = "Some ViewTxt";
    st->frame.events.render = _ViewTxt_render;
    st->frame.events.closing = frame_close; // ZZZ

    res->data = st;

    puts("== creating frame in background thread");

    if (pthread_create(&st->thread, NULL, (void*(*)(void*))_ViewTxt_thread, &st->frame)
            || pthread_detach(st->thread)) {
        free(st);
        fail("could not create background thread");
    }

    _frame_ok_ok = false;
    pthread_mutex_lock(&_frame_ok_mx);
    while (!_frame_ok_ok) pthread_cond_wait(&_frame_ok_cv, &_frame_ok_mx);
    pthread_mutex_unlock(&_frame_ok_mx);

    if (!_frame_ok_st) {
        free(st);
        fail("could not create window");
    }

    return true;
}

bool _ViewTxt(Buf* self, Buf const* const txt) {
    ViewTxtState* st = getdata(self);

    if (destroyed(self)) {
        puts("== frame close request");
        if (0 == pthread_cancel(st->thread)) frame_redraw(&st->frame);
        puts("== request sent");
        sleep(1);
        puts("== done sleeping");
        free(st);
        return true;
    }

    //if (!edited) ..
    *self = *txt;
    return true;
}
