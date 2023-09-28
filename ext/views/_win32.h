#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

struct Frame { extends_FrameBase;
    HWND hWnd;
    HDC hDC;
    HGLRC hRC;
};

#ifdef FRAME_IMPLEMENTATION
#include <windowsx.h>
#include <GL/gl.h>

LRESULT CALLBACK _WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Frame* self = (Frame*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (self && hWnd == self->hWnd) switch (uMsg) {
        case WM_PAINT: {
            _event(render, self);
            SwapBuffers(self->hDC);
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
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

#define KEY_BACKSPACE  0x08 // BACKSPACE key
#define KEY_TAB        0x09 // TAB key
#define KEY_RETURN     0x0D // ENTER key
#define KEY_LSHIFT     0x10 // LEFT SHIFT key
#define KEY_RSHIFT     0x10 // RIGHT SHIFT key
#define KEY_LCTRL      0x11 // LEFT CTRL key
#define KEY_RCTRL      0x11 // RIGHT CTRL key
#define KEY_CAPLOCK    0x14 // CAPS LOCK key
#define KEY_ESCAPE     0x1B // ESC key
#define KEY_SPACE      0x20 // SPACEBAR
#define KEY_PAGEUP     0x21 // PAGE UP key
#define KEY_PAGEDOWN   0x22 // PAGE DOWN key
#define KEY_END        0x23 // END key
#define KEY_HOME       0x24 // HOME key
#define KEY_LEFT       0x25 // LEFT ARROW key
#define KEY_UP         0x26 // UP ARROW key
#define KEY_RIGHT      0x27 // RIGHT ARROW key
#define KEY_DOWN       0x28 // DOWN ARROW key
#define KEY_INSERT     0x2D // INS key
#define KEY_DELETE     0x2E // DEL key
#define KEY_0          0x30 // 0 key
#define KEY_1          0x31 // 1 key
#define KEY_2          0x32 // 2 key
#define KEY_3          0x33 // 3 key
#define KEY_4          0x34 // 4 key
#define KEY_5          0x35 // 5 key
#define KEY_6          0x36 // 6 key
#define KEY_7          0x37 // 7 key
#define KEY_8          0x38 // 8 key
#define KEY_9          0x39 // 9 key
#define KEY_A          0x41 // A key
#define KEY_B          0x42 // B key
#define KEY_C          0x43 // C key
#define KEY_D          0x44 // D key
#define KEY_E          0x45 // E key
#define KEY_F          0x46 // F key
#define KEY_G          0x47 // G key
#define KEY_H          0x48 // H key
#define KEY_I          0x49 // I key
#define KEY_J          0x4A // J key
#define KEY_K          0x4B // K key
#define KEY_L          0x4C // L key
#define KEY_M          0x4D // M key
#define KEY_N          0x4E // N key
#define KEY_O          0x4F // O key
#define KEY_P          0x50 // P key
#define KEY_Q          0x51 // Q key
#define KEY_R          0x52 // R key
#define KEY_S          0x53 // S key
#define KEY_T          0x54 // T key
#define KEY_U          0x55 // U key
#define KEY_V          0x56 // V key
#define KEY_W          0x57 // W key
#define KEY_X          0x58 // X key
#define KEY_Y          0x59 // Y key
#define KEY_Z          0x5A // Z key
#define KEY_MENU       0x5D // Applications key
#define KEY_NUMPAD0    0x60 // Numeric keypad 0 key
#define KEY_NUMPAD1    0x61 // Numeric keypad 1 key
#define KEY_NUMPAD2    0x62 // Numeric keypad 2 key
#define KEY_NUMPAD3    0x63 // Numeric keypad 3 key
#define KEY_NUMPAD4    0x64 // Numeric keypad 4 key
#define KEY_NUMPAD5    0x65 // Numeric keypad 5 key
#define KEY_NUMPAD6    0x66 // Numeric keypad 6 key
#define KEY_NUMPAD7    0x67 // Numeric keypad 7 key
#define KEY_NUMPAD8    0x68 // Numeric keypad 8 key
#define KEY_NUMPAD9    0x69 // Numeric keypad 9 key
#define KEY_MULTIPLY   0x6A // Multiply key
#define KEY_ADD        0x6B // Add key
#define KEY_SUBTRACT   0x6D // Subtract key
#define KEY_DECIMAL    0x6E // Decimal key
#define KEY_DIVIDE     0x6F // Divide key
#define KEY_F1         0x70 // F1 key
#define KEY_F2         0x71 // F2 key
#define KEY_F3         0x72 // F3 key
#define KEY_F4         0x73 // F4 key
#define KEY_F5         0x74 // F5 key
#define KEY_F6         0x75 // F6 key
#define KEY_F7         0x76 // F7 key
#define KEY_F8         0x77 // F8 key
#define KEY_F9         0x78 // F9 key
#define KEY_F10        0x79 // F10 key
#define KEY_F11        0x7A // F11 key
#define KEY_F12        0x7B // F12 key
#define KEY_F13        0x7C // F13 key
#define KEY_F14        0x7D // F14 key
#define KEY_F15        0x7E // F15 key
#define KEY_F16        0x7F // F16 key
#define KEY_F17        0x80 // F17 key
#define KEY_F18        0x81 // F18 key
#define KEY_F19        0x82 // F19 key
#define KEY_F20        0x83 // F20 key
#define KEY_F21        0x84 // F21 key
#define KEY_F22        0x85 // F22 key
#define KEY_F23        0x86 // F23 key
#define KEY_F24        0x87 // F24 key
