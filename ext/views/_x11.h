#include <GL/glx.h>

struct Frame { extends_FrameBase
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
    Atom wm_protocols = XInternAtom(self->dpy, "WM_PROTOCOLS", true);
    Atom wm_frame_redraw = XInternAtom(self->dpy, "_FRAME_WM_REDRAW_WINDOW", false);
    Atom wm_frame_close = XInternAtom(self->dpy, "_FRAME_WM_CLOSE_WINDOW", false);
    Atom wm_delete = XInternAtom(self->dpy, "WM_DELETE_WINDOW", false);
    XSetWMProtocols(self->dpy, self->win, &wm_delete, 1);

    XEvent xev;
    while (true) {
        XNextEvent(self->dpy, &xev);

        switch (xev.type) {
            case Expose: {
                int ww = xev.xexpose.width;
                int hh = xev.xexpose.height;
                if (1 < ww && 1 < hh) {
                    glViewport(0, 0, ww, hh);
                    _event(resize, self, ww, hh);
                    self->width = ww;
                    self->height = hh;
                    _event(render, self);
                    //glXSwapBuffers(self->dpy, self->win); // XXX: idk
                }
             } break;

            case KeyPress:   _event(keydown, self, xev.xkey.keycode); break;
            case KeyRelease: _event(keyup,   self, xev.xkey.keycode); break;

            case ButtonPress:
                if (4 == xev.xbutton.button || 5 == xev.xbutton.button)
                    _event(mousewheel, self, 1-(xev.xbutton.button-4)*2, xev.xbutton.x, xev.xbutton.y);
                else
                    _event(mousedown, self, xev.xbutton.button, xev.xbutton.x, xev.xbutton.y);
                break;
            case ButtonRelease: _event(mouseup,   self, xev.xbutton.button, xev.xbutton.x, xev.xbutton.y); break;
            case MotionNotify:  _event(mousemove, self, xev.xmotion.x, xev.xmotion.y); break;

            case ClientMessage:
                if (wm_protocols == xev.xclient.message_type) {
                    if (wm_frame_redraw == (Atom)xev.xclient.data.l[0]) {
                        _event(render, self);
                        glXSwapBuffers(self->dpy, self->win);
                        break;
                    }
                    if (wm_delete == (Atom)xev.xclient.data.l[0]) {
                        _event(closing, self);
                        break;
                    }
                    if (wm_frame_close == (Atom)xev.xclient.data.l[0])
                        return;
                }
        }
    } // while true
}

void frame_redraw(Frame* self) {
    static bool once = false;
    static Atom wm_protocols = 0;
    static Atom wm_frame_redraw = 0;
    if (!once) {
        once = true;
        wm_protocols = XInternAtom(self->dpy, "WM_PROTOCOLS", true);
        wm_frame_redraw = XInternAtom(self->dpy, "_FRAME_WM_REDRAW_WINDOW", false);
    }

    XEvent event;
    event.xclient.type = ClientMessage;
    event.xclient.window = self->win;
    event.xclient.message_type = wm_protocols;
    event.xclient.format = 32;
    event.xclient.data.l[0] = wm_frame_redraw;
    event.xclient.data.l[1] = CurrentTime;
    XSendEvent(self->dpy, self->win, False, NoEventMask, &event);
}

void frame_close(Frame* self) {
    static bool once = false;
    static Atom wm_protocols = 0;
    static Atom wm_frame_close = 0;
    if (!once) {
        once = true;
        wm_protocols = XInternAtom(self->dpy, "WM_PROTOCOLS", true);
        wm_frame_close = XInternAtom(self->dpy, "_FRAME_WM_CLOSE_WINDOW", false);
    }

    XEvent event;
    event.xclient.type = ClientMessage;
    event.xclient.window = self->win;
    event.xclient.message_type = wm_protocols;
    event.xclient.format = 32;
    event.xclient.data.l[0] = wm_frame_close;
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

#define MOUSE_LEFT   1
#define MOUSE_RIGHT  3
#define MOUSE_MIDDLE 2

// TODO: these are not correct everywhere
#define KEY_BACKSPACE  0x16 // BACKSPACE key
#define KEY_TAB        0x17 // TAB key
#define KEY_RETURN     0x24 // ENTER key
#define KEY_LSHIFT     0x32 // LEFT SHIFT key
#define KEY_RSHIFT     0x3E // RIGHT SHIFT key
#define KEY_LCTRL      0x25 // LEFT CTRL key
#define KEY_RCTRL      0x69 // RIGHT CTRL key
#define KEY_CAPLOCK    0x42 // CAPS LOCK key
#define KEY_ESC        0x09 // ESC key
#define KEY_SPACE      0x41 // SPACEBAR
#define KEY_PAGEUP     0x70 // PAGE UP key
#define KEY_PAGEDOWN   0x75 // PAGE DOWN key
#define KEY_END        0x73 // END key
#define KEY_HOME       0x6E // HOME key
#define KEY_LEFT       0x71 // LEFT ARROW key
#define KEY_UP         0x6F // UP ARROW key
#define KEY_RIGHT      0x72 // RIGHT ARROW key
#define KEY_DOWN       0x74 // DOWN ARROW key
#define KEY_INSERT     0x76 // INS key
#define KEY_DELETE     0x77 // DEL key
#define KEY_0          0x13 // 0 key
#define KEY_1          0x0A // 1 key
#define KEY_2          0x0B // 2 key
#define KEY_3          0x0C // 3 key
#define KEY_4          0x0D // 4 key
#define KEY_5          0x0E // 5 key
#define KEY_6          0x0F // 6 key
#define KEY_7          0x10 // 7 key
#define KEY_8          0x11 // 8 key
#define KEY_9          0x12 // 9 key
#define KEY_A          0x26 // A key
#define KEY_B          0x38 // B key
#define KEY_C          0x36 // C key
#define KEY_D          0x28 // D key
#define KEY_E          0x1A // E key
#define KEY_F          0x29 // F key
#define KEY_G          0x2A // G key
#define KEY_H          0x2B // H key
#define KEY_I          0x1F // I key
#define KEY_J          0x2C // J key
#define KEY_K          0x2D // K key
#define KEY_L          0x2E // L key
#define KEY_M          0x3A // M key
#define KEY_N          0x39 // N key
#define KEY_O          0x20 // O key
#define KEY_P          0x21 // P key
#define KEY_Q          0x18 // Q key
#define KEY_R          0x1B // R key
#define KEY_S          0x27 // S key
#define KEY_T          0x1C // T key
#define KEY_U          0x1E // U key
#define KEY_V          0x37 // V key
#define KEY_W          0x19 // W key
#define KEY_X          0x35 // X key
#define KEY_Y          0x1D // Y key
#define KEY_Z          0x34 // Z key
#define KEY_MENU            // Applications key
#define KEY_NUMPAD0         // Numeric keypad 0 key
#define KEY_NUMPAD1         // Numeric keypad 1 key
#define KEY_NUMPAD2         // Numeric keypad 2 key
#define KEY_NUMPAD3         // Numeric keypad 3 key
#define KEY_NUMPAD4         // Numeric keypad 4 key
#define KEY_NUMPAD5         // Numeric keypad 5 key
#define KEY_NUMPAD6         // Numeric keypad 6 key
#define KEY_NUMPAD7         // Numeric keypad 7 key
#define KEY_NUMPAD8         // Numeric keypad 8 key
#define KEY_NUMPAD9         // Numeric keypad 9 key
#define KEY_MULTIPLY        // Multiply key
#define KEY_ADD             // Add key
#define KEY_SUBTRACT        // Subtract key
#define KEY_DECIMAL         // Decimal key
#define KEY_DIVIDE          // Divide key
#define KEY_F1         0x43 // F1 key
#define KEY_F2         0x44 // F2 key
#define KEY_F3         0x45 // F3 key
#define KEY_F4         0x46 // F4 key
#define KEY_F5         0x47 // F5 key
#define KEY_F6         0x48 // F6 key
#define KEY_F7         0x49 // F7 key
#define KEY_F8         0x4A // F8 key
#define KEY_F9         0x4B // F9 key
#define KEY_F10        0x4C // F10 key
#define KEY_F11        0x5F // F11 key
#define KEY_F12        0x60 // F12 key
#define KEY_F13             // F13 key
#define KEY_F14             // F14 key
#define KEY_F15             // F15 key
#define KEY_F16             // F16 key
#define KEY_F17             // F17 key
#define KEY_F18             // F18 key
#define KEY_F19             // F19 key
#define KEY_F20             // F20 key
#define KEY_F21             // F21 key
#define KEY_F22             // F22 key
#define KEY_F23             // F23 key
#define KEY_F24             // F24 key
