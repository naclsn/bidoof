#include <GL/glx.h>

struct Frame { extends_FrameBase;
    Display* dpy;
    Window win;
    GLXContext glc;
};

#ifdef FRAME_IMPLEMENTATION
#include <stdio.h>

static volatile unsigned char _errreceived = 0;
int _errhandler(Display* dpy, XErrorEvent* err) {
    _errreceived = err->error_code;
    return 0;
}

bool frame_create(Frame* self, char const* title) {
    // only registers the error handler once
    static bool handling = false;
    if (!handling) {
        handling = true;
        XSetErrorHandler(_errhandler);
    }

    if (!(self->dpy = XOpenDisplay(NULL))) return false;
    Window root = DefaultRootWindow(self->dpy);

    GLint att[] = { GLX_RGBA, GLX_DOUBLEBUFFER, None };
    XVisualInfo* vi = glXChooseVisual(self->dpy, 0, att);
    if (!vi) {
        XCloseDisplay(self->dpy);
        return false;
    }

    _errreceived = 0;
    Colormap cmap = XCreateColormap(self->dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa = {
        .event_mask= ExposureMask | KeyPressMask,
        .colormap= cmap,
    };

    if (!_errreceived
            && (self->win = XCreateWindow(self->dpy, root,
                    0, 0, self->width, self->height,
                    0, vi->depth, InputOutput, vi->visual,
                    CWColormap | CWEventMask, &swa),
                !_errreceived)
            && (XMapWindow(self->dpy, self->win),
                XStoreName(self->dpy, self->win, title),
                !_errreceived)
            && (self->glc = glXCreateContext(self->dpy, vi, NULL, GL_TRUE))
            && (glXMakeCurrent(self->dpy, self->win, self->glc),
                !_errreceived)
       ) return true;

    frame_destroy(self);
    return true;
}

void frame_loop(Frame* self) {
    XEvent xev;
    XWindowAttributes xwa;
    while (true) {
        XNextEvent(self->dpy, &xev);

        switch (xev.type) {
            case Expose:
                XGetWindowAttributes(self->dpy, self->win, &xwa);
                glViewport(0, 0, xwa.width, xwa.height);
                _event(render, self);
                glXSwapBuffers(self->dpy, self->win);
                break;

            case KeyPress:
                if (9 == xev.xkey.keycode) // escape
                    frame_close(self);
                break;

            case ClientMessage:
                //if (..)
                    return;
        }
    }
}

void frame_redraw(Frame* self) {
    XEvent event;
    event.xclient.type = ClientMessage;
    event.xclient.window = self->win;
    event.xclient.message_type = XInternAtom(self->dpy, "WM_PROTOCOLS", true);
    event.xclient.format = 32;
    event.xclient.data.l[0] = XInternAtom(self->dpy, "WM_REDRAW_WINDOW", false);
    event.xclient.data.l[1] = CurrentTime;
    XSendEvent(self->dpy, self->win, False, NoEventMask, &event);
}

void frame_close(Frame* self) {
    XEvent event;
    event.xclient.type = ClientMessage;
    event.xclient.window = self->win;
    event.xclient.message_type = XInternAtom(self->dpy, "WM_PROTOCOLS", true);
    event.xclient.format = 32;
    event.xclient.data.l[0] = XInternAtom(self->dpy, "WM_DELETE_WINDOW", false);
    event.xclient.data.l[1] = CurrentTime;
    XSendEvent(self->dpy, self->win, False, NoEventMask, &event);
}

void frame_destroy(Frame* self) {
    if (self->dpy) {
        glXMakeCurrent(self->dpy, None, NULL);
        if (self->glc) glXDestroyContext(self->dpy, self->glc);
        if (self->win) XDestroyWindow(self->dpy, self->win);
        XCloseDisplay(self->dpy);
    }

    self->dpy = NULL;
    self->win = 0;
    self->glc = 0;
}

#endif // FRAME_IMPLEMENTATION
