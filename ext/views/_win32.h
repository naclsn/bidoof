#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

struct FrameImpl {
    HWND hWnd;
    HDC hDC;
    HGLRC hRC;
};

#ifdef FRAME_IMPLEMENTATION
#include <windowsx.h>
#include <GL/gl.h>

LONG WINAPI _WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Frame* self = (Frame*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (self && hWnd == self->_impl.hWnd) switch (uMsg) {
        case WM_PAINT: {
            _event(self, render)(self);
            SwapBuffers(self->_impl.hDC);
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        } return 0;

        case WM_SIZE: {
            WORD ww = LOWORD(lParam), hh = HIWORD(lParam);
            _event(self, resize)(self, ww, hh);
            self->width = ww;
            self->height = hh;
            glViewport(0, 0, ww, hh);
            PostMessage(hWnd, WM_PAINT, 0, 0);
        } return 0;

        case WM_CLOSE:          _event(self, closing)(self); return 0;
        case WM_KEYDOWN:        _event(self, keydown)(self, wParam); return 0;
        case WM_KEYUP:          _event(self, keyup)(self, wParam); return 0;
        case WM_CHAR:           _event(self, keychar)(self, wParam); return 0;
        case WM_LBUTTONDOWN:    _event(self, mousedown)(self, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
        case WM_LBUTTONUP:      _event(self, mouseup)(self, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
        case WM_LBUTTONDBLCLK:  _event(self, mousedouble)(self, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
        case WM_RBUTTONDOWN:    _event(self, mousedown)(self, 1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
        case WM_RBUTTONUP:      _event(self, mouseup)(self, 1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
        case WM_RBUTTONDBLCLK:  _event(self, mousedouble)(self, 1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
        case WM_MBUTTONDOWN:    _event(self, mousedown)(self, 2, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
        case WM_MBUTTONUP:      _event(self, mouseup)(self, 2, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
        case WM_MBUTTONDBLCLK:  _event(self, mousedouble)(self, 2, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
        case WM_MOUSEWHEEL:     _event(self, mousewheel)(self, GET_WHEEL_DELTA_WPARAM(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
        case WM_MOUSEMOVE:      _event(self, mousemove)(self, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool frame_create(Frame* self, char const* title) {
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
    self->_impl.hWnd = CreateWindow("OpenGLFrame", title,
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

    if (self->_impl.hWnd
            && (SetWindowLongPtr(self->_impl.hWnd, GWLP_USERDATA, (LONG_PTR)self), !GetLastError())
            && (self->_impl.hDC = GetDC(self->hWnd))
            && (pf = ChoosePixelFormat(self->_impl.hDC, &pfd))
            && DescribePixelFormat(self->_impl.hDC, pf, sizeof pfd, &pfd)
            && SetPixelFormat(self->_impl.hDC, pf, &pfd)
            && (self->_impl.hRC = wglCreateContext(self->hDC))
            && wglMakeCurrent(self->_impl.hDC, self->hRC)
       ) {
        ShowWindow(self->_impl.hWnd, SW_SHOW);
        return true;
    }

    frame_destroy(self);
    return false;
}

void frame_loop(Frame* self) {
    MSG msg;
    while (0 < GetMessage(&msg, self->_impl.hWnd, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void frame_close(Frame* self) {
    DestroyWindow(self->_impl.hWnd);
}

void frame_destroy(Frame* self) {
    if (self->_impl.hRC) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(self->_impl.hRC);
    }
    if (self->_impl.hDC) ReleaseDC(self->hWnd, self->hDC);
    DestroyWindow(self->_impl.hWnd);

    self->_impl.hWnd = NULL;
    self->_impl.hDC = NULL;
    self->_impl.hRC = NULL;
}

#endif // FRAME_IMPLEMENTATION
