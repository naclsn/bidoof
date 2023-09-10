#include <GL/glx.h>

struct Frame { extends_FrameBase;
    Display* dpy;
    Window win;
    GLXContext glc;
};

#ifdef FRAME_IMPLEMENTATION

static volatile unsigned char _errreceived = 0;
int _errhandler(Display* dpy, XErrorEvent* err) {
    (void)dpy;
    _errreceived = err->error_code;
    return 0;
}

bool frame_create(Frame* self) {
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
        .event_mask= ExposureMask
                   | KeyPressMask
                   //| ResizeRedirectMask
                   | KeyPressMask
                   | KeyReleaseMask
                   | ButtonPressMask
                   | ButtonReleaseMask
                   | PointerMotionMask,
        .colormap= cmap,
    };

    if (!_errreceived
            && (self->win = XCreateWindow(self->dpy, root,
                    0, 0, self->width, self->height,
                    0, vi->depth, InputOutput, vi->visual,
                    CWColormap | CWEventMask, &swa),
                !_errreceived)
            && (XMapWindow(self->dpy, self->win),
                XStoreName(self->dpy, self->win, self->title),
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
    while (true) {
        XNextEvent(self->dpy, &xev);

        switch (xev.type) {
            case Expose: {
                int ww = xev.xexpose.width;
                int hh = xev.xexpose.height;
                glViewport(0, 0, ww, hh);
                _event(resize, self, ww, hh);
                self->width = ww;
                self->height = hh;
                _event(render, self);
                glXSwapBuffers(self->dpy, self->win);
             } break;

            //case ResizeRequest: {
            //    int ww = xev.xresizerequest.width;
            //    int hh = xev.xresizerequest.height;
            //    _event(resize, self, ww, hh);
            //    self->width = ww;
            //    self->height = hh;
            //    glViewport(0, 0, ww, hh);
            //    _event(render, self);
            //    glXSwapBuffers(self->dpy, self->win);
            //} break;

            //case TODO: _event(closing, self); break;

            case KeyPress:      _event(keydown, self, xev.xkey.keycode); break;
            case KeyRelease:    _event(keyup,   self, xev.xkey.keycode); break;

            case ButtonPress:
                if (4 == xev.xbutton.button || 5 == xev.xbutton.button)
                    _event(mousewheel, self, 1-(xev.xbutton.button-4)*2, xev.xbutton.x, xev.xbutton.y);
                else
                    _event(mousedown, self, xev.xbutton.button, xev.xbutton.x, xev.xbutton.y);
                break;
            case ButtonRelease: _event(mouseup,   self, xev.xbutton.button, xev.xbutton.x, xev.xbutton.y); break;
            case MotionNotify:  _event(mousemove, self, xev.xmotion.x, xev.xmotion.y); break;

            case ClientMessage:
                if (XInternAtom(self->dpy, "WM_PROTOCOLS", true) ==  xev.xclient.message_type) {
                    if (XInternAtom(self->dpy, "WM_REDRAW_WINDOW", false) == (unsigned long)xev.xclient.data.l[0]) {
                        _event(render, self);
                        glXSwapBuffers(self->dpy, self->win);
                        break;
                    }
                    if (XInternAtom(self->dpy, "WM_DELETE_WINDOW", false) == (unsigned long)xev.xclient.data.l[0]) return;
                }
        }
    } // while true
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
