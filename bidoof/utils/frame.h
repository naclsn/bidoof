#ifndef __FRAME_H__
#define __FRAME_H__

/// See the end of the file for an example; the idea is as follow:
/// - create a `Frame` structure with a size, title and events
/// - call `frame_create` (it returns false on failure)
/// - then `frame_loop` to make it live, this is blocking
/// - finally `frame_destroy` when it falls out of it
///
/// Calling `frame_redraw` forces a render event.
/// Calling `frame_close` makes it fall out of the loop.
/// I have no clue as for the thread-safety of anything.
///
/// Make sure to only use FRAME_IMPLEMENTATION once.
/// (TODO: make it possible to select the impl with the macro.)
/// If any of _WIN32, _WIN64 or __CYGWIN__ is present, uses Win32 API.
/// Otherwise, only X11 is supported for now.

#include <stdbool.h>

typedef struct Frame Frame;

bool frame_create(Frame* self);
void frame_loop(Frame* self);
void frame_redraw(Frame* self);
void frame_close(Frame* self);
void frame_destroy(Frame* self);

unsigned frame_key2char(unsigned key);

#define extends_FrameBase                                            \
    int width;                                                       \
    int height;                                                      \
    char const* title;                                               \
                                                                     \
    struct {                                                         \
        void (*render)(Frame* self);                                 \
        void (*resize)(Frame* self, int w, int h);                   \
        void (*closing)(Frame* self);                                \
                                                                     \
        void (*keydown)(Frame* self, unsigned key);                  \
        void (*keyup)(Frame* self, unsigned key);                    \
                                                                     \
        void (*mousedown)(Frame* self, int button, int x, int y);    \
        void (*mouseup)(Frame* self, int button, int x, int y);      \
                                                                     \
        void (*mousewheel)(Frame* self, int delta, int x, int y);    \
        void (*mousemove)(Frame* self, int x, int y);                \
    } events;                                                        \

#define _event_frame(__frame, ...) __frame
#define _event(__name, ...) do                                   \
    if (_event_frame(__VA_ARGS__,)->events.__name)               \
        _event_frame(__VA_ARGS__,)->events.__name(__VA_ARGS__);  \
    while (false)

#if defined _WIN32 || defined _WIN64 || defined __CYGWIN__

#include "_win32.h"
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX
#include <Windows.h>

struct Frame { extends_FrameBase
    HWND hWnd;
    HDC hDC;
    HGLRC hRC;
};

#ifdef FRAME_IMPLEMENTATION
#include <Windowsx.h>
#include <GL/gl.h>

LRESULT CALLBACK _WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Frame* self = (Frame*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (self && hWnd == self->hWnd) switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            if (ps.rcPaint.right - ps.rcPaint.left || ps.rcPaint.bottom - ps.rcPaint.top) {
                _event(render, self);
                SwapBuffers(self->hDC);
            }
            EndPaint(hWnd, &ps);
        } return 0;

        case WM_SIZE: {
            WORD ww = LOWORD(lParam), hh = HIWORD(lParam);
            _event(resize, self, ww, hh);
            self->width = ww;
            self->height = hh;
            glViewport(0, 0, ww, hh);
            PostMessage(hWnd, WM_PAINT, 0, 0);
        } return 0;

        case WM_CLOSE:          _event(closing,     self); return 0;
        case WM_KEYDOWN:        _event(keydown,     self, wParam); return 0;
        case WM_KEYUP:          _event(keyup,       self, wParam); return 0;
        case WM_LBUTTONDOWN:    _event(mousedown,   self, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
        case WM_LBUTTONUP:      _event(mouseup,     self, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
        case WM_RBUTTONDOWN:    _event(mousedown,   self, 1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
        case WM_RBUTTONUP:      _event(mouseup,     self, 1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
        case WM_MBUTTONDOWN:    _event(mousedown,   self, 2, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
        case WM_MBUTTONUP:      _event(mouseup,     self, 2, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
        case WM_MOUSEWHEEL:     _event(mousewheel,  self, GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
        case WM_MOUSEMOVE:      _event(mousemove,   self, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool frame_create(Frame* self) {
    // only registers the window class once
    static HINSTANCE hInstance = NULL;
    if (!hInstance) {
        hInstance = GetModuleHandle(NULL);

        WNDCLASS wc = {
            .style= CS_OWNDC,
            .lpfnWndProc= (WNDPROC)_WindowProc,
            .cbClsExtra= 0,
            .cbWndExtra= 0,
            .hInstance= hInstance,
            .hIcon= LoadIcon(NULL, IDI_WINLOGO),
            .hCursor= LoadCursor(NULL, IDC_ARROW),
            .hbrBackground= NULL,
            .lpszMenuName= NULL,
            .lpszClassName= "OpenGLFrame",
        };
        if (!RegisterClass(&wc)) {
            hInstance = NULL;
            return false;
        }
    }

    SetLastError(0);
    self->hWnd = CreateWindow("OpenGLFrame", self->title,
            WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            CW_USEDEFAULT, 0, self->width, self->height,
            NULL, NULL, hInstance, NULL);

    PIXELFORMATDESCRIPTOR pfd = {
        .nSize= sizeof pfd,
        .nVersion= 1,
        .dwFlags= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType= PFD_TYPE_RGBA,
        .cColorBits= 32,
    };
    int pf;

    if (self->hWnd
            && (SetWindowLongPtr(self->hWnd, GWLP_USERDATA, (LONG_PTR)self), !GetLastError())
            && (self->hDC = GetDC(self->hWnd))
            && (pf = ChoosePixelFormat(self->hDC, &pfd))
            && DescribePixelFormat(self->hDC, pf, sizeof pfd, &pfd)
            && SetPixelFormat(self->hDC, pf, &pfd)
            && (self->hRC = wglCreateContext(self->hDC))
            && wglMakeCurrent(self->hDC, self->hRC)
       ) {
        ShowWindow(self->hWnd, SW_SHOW);
        return true;
    }

    frame_destroy(self);
    return false;
}

void frame_loop(Frame* self) {
    MSG msg;
    while (0 < GetMessage(&msg, self->hWnd, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void frame_redraw(Frame* self) {
    InvalidateRect(self->hWnd, NULL, TRUE);
    PostMessage(self->hWnd, WM_PAINT, 0, 0);
}

void frame_close(Frame* self) {
    DestroyWindow(self->hWnd);
}

void frame_destroy(Frame* self) {
    if (self->hWnd) {
        wglMakeCurrent(NULL, NULL);
        if (self->hRC) wglDeleteContext(self->hRC);
        if (self->hDC) ReleaseDC(self->hWnd, self->hDC);
        DestroyWindow(self->hWnd);
    }

    self->hWnd = NULL;
    self->hDC = NULL;
    self->hRC = NULL;
}

#endif // FRAME_IMPLEMENTATION

#define MOUSE_LEFT   0
#define MOUSE_RIGHT  1
#define MOUSE_MIDDLE 2

#else // win32

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

            // TODO:
            // int dummy;
            // KeySym* keysyms = XGetKeyboardMapping(self->dpy, scancode, 1, &dummy);
            // SOKOL_ASSERT(keysyms);
            // KeySym keysym = keysyms[0];
            // XFree(keysyms);
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

#endif // x11

#undef _event_frame
#undef _event
#undef extends_FrameBase

#if 0 // example

#define FRAME_IMPLEMENTATION
#include "(this file)"
#include <pthread.h> // if using main_threaded
#include <unistd.h> // sleep

void triangle(Frame* frame) {
    pthread_testcancel(); // if using main_threaded
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
Frame frame = {
    .width= 256,
    .height= 256,
    .title= "wow! window! crazy!",
    .events= {
        .render= triangle,
        .closing= frame_close,
    },
};

void* myframe_main(Frame* frame) {
    pthread_cleanup_push((void*)frame_destroy, frame);
        if (frame_create(frame)) frame_loop(frame);
    pthread_cleanup_pop(true);
    return NULL;
}
int main_threaded(void) {
    pthread_t thr;
    int r;
    if ((r = pthread_create(&thr, NULL, (void*)myframe_main, &frame)) || (r = pthread_detach(thr))) return r;
    sleep(5); // sleep a bit in the mean time
    if (!pthread_cancel(thr)) frame_redraw(&frame); // force a cancelation point (win32 event loop isn't one :/)
    sleep(1); // give time to reach the cancelation point
    return 0;
}

int main_nothread(void) {
    if (!frame_create(&frame)) return 1;
    frame_loop(&frame);
    frame_destroy(&frame);
    return 0;
}

#endif // example

#endif // __FRAME_H__
