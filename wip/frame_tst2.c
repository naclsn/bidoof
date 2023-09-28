#include <stdio.h>
#include <string.h>

#define FRAME_IMPLEMENTATION
#include "../ext/views/frame.h"
#define TEXT_IMPLEMENTATION
#include "../ext/views/text.h"

void render(Frame* f) {
    (void)f;
    glClearColor(.12f, .15f, .18f, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor4f(.88f, .66f, .77f, 1.f);
    char const* const txt = "hi and such\r\n\u3057\u304A";
    text_draw(txt, strlen(txt), 10, f->height-8*2.f -10, 2.f);
}

void resize(Frame* f, int w, int h) {
    (void)f;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, w, 0.0, h, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
}

void keydown(Frame* f, unsigned key) {
    (void)f;
    switch (key) {
        case KEY_BACKSPACE:  printf("KEY_BACKSPACE");  break;
        case KEY_TAB:        printf("KEY_TAB");        break;
        case KEY_RETURN:     printf("KEY_RETURN");     break;
        case KEY_LSHIFT:     printf("KEY_LSHIFT");     break;
        case KEY_RSHIFT:     printf("KEY_RSHIFT");     break;
        case KEY_LCTRL:      printf("KEY_LCTRL");      break;
        case KEY_RCTRL:      printf("KEY_RCTRL");      break;
        case KEY_CAPLOCK:    printf("KEY_CAPLOCK");    break;
        case KEY_ESCAPE:     printf("KEY_ESCAPE");     break;
        case KEY_SPACE:      printf("KEY_SPACE");      break;
        case KEY_PAGEUP:     printf("KEY_PAGEUP");     break;
        case KEY_PAGEDOWN:   printf("KEY_PAGEDOWN");   break;
        case KEY_END:        printf("KEY_END");        break;
        case KEY_HOME:       printf("KEY_HOME");       break;
        case KEY_LEFT:       printf("KEY_LEFT");       break;
        case KEY_UP:         printf("KEY_UP");         break;
        case KEY_RIGHT:      printf("KEY_RIGHT");      break;
        case KEY_DOWN:       printf("KEY_DOWN");       break;
        case KEY_INSERT:     printf("KEY_INSERT");     break;
        case KEY_DELETE:     printf("KEY_DELETE");     break;
        case KEY_0:          printf("KEY_0");          break;
        case KEY_1:          printf("KEY_1");          break;
        case KEY_2:          printf("KEY_2");          break;
        case KEY_3:          printf("KEY_3");          break;
        case KEY_4:          printf("KEY_4");          break;
        case KEY_5:          printf("KEY_5");          break;
        case KEY_6:          printf("KEY_6");          break;
        case KEY_7:          printf("KEY_7");          break;
        case KEY_8:          printf("KEY_8");          break;
        case KEY_9:          printf("KEY_9");          break;
        case KEY_A:          printf("KEY_A");          break;
        case KEY_B:          printf("KEY_B");          break;
        case KEY_C:          printf("KEY_C");          break;
        case KEY_D:          printf("KEY_D");          break;
        case KEY_E:          printf("KEY_E");          break;
        case KEY_F:          printf("KEY_F");          break;
        case KEY_G:          printf("KEY_G");          break;
        case KEY_H:          printf("KEY_H");          break;
        case KEY_I:          printf("KEY_I");          break;
        case KEY_J:          printf("KEY_J");          break;
        case KEY_K:          printf("KEY_K");          break;
        case KEY_L:          printf("KEY_L");          break;
        case KEY_M:          printf("KEY_M");          break;
        case KEY_N:          printf("KEY_N");          break;
        case KEY_O:          printf("KEY_O");          break;
        case KEY_P:          printf("KEY_P");          break;
        case KEY_Q:          printf("KEY_Q");          break;
        case KEY_R:          printf("KEY_R");          break;
        case KEY_S:          printf("KEY_S");          break;
        case KEY_T:          printf("KEY_T");          break;
        case KEY_U:          printf("KEY_U");          break;
        case KEY_V:          printf("KEY_V");          break;
        case KEY_W:          printf("KEY_W");          break;
        case KEY_X:          printf("KEY_X");          break;
        case KEY_Y:          printf("KEY_Y");          break;
        case KEY_Z:          printf("KEY_Z");          break;
        //case KEY_MENU:       printf("KEY_MENU");       break;
        //case KEY_NUMPAD0:    printf("KEY_NUMPAD0");    break;
        //case KEY_NUMPAD1:    printf("KEY_NUMPAD1");    break;
        //case KEY_NUMPAD2:    printf("KEY_NUMPAD2");    break;
        //case KEY_NUMPAD3:    printf("KEY_NUMPAD3");    break;
        //case KEY_NUMPAD4:    printf("KEY_NUMPAD4");    break;
        //case KEY_NUMPAD5:    printf("KEY_NUMPAD5");    break;
        //case KEY_NUMPAD6:    printf("KEY_NUMPAD6");    break;
        //case KEY_NUMPAD7:    printf("KEY_NUMPAD7");    break;
        //case KEY_NUMPAD8:    printf("KEY_NUMPAD8");    break;
        //case KEY_NUMPAD9:    printf("KEY_NUMPAD9");    break;
        //case KEY_MULTIPLY:   printf("KEY_MULTIPLY");   break;
        //case KEY_ADD:        printf("KEY_ADD");        break;
        //case KEY_SUBTRACT:   printf("KEY_SUBTRACT");   break;
        //case KEY_DECIMAL:    printf("KEY_DECIMAL");    break;
        //case KEY_DIVIDE:     printf("KEY_DIVIDE");     break;
        case KEY_F1:         printf("KEY_F1");         break;
        case KEY_F2:         printf("KEY_F2");         break;
        case KEY_F3:         printf("KEY_F3");         break;
        case KEY_F4:         printf("KEY_F4");         break;
        case KEY_F5:         printf("KEY_F5");         break;
        case KEY_F6:         printf("KEY_F6");         break;
        case KEY_F7:         printf("KEY_F7");         break;
        case KEY_F8:         printf("KEY_F8");         break;
        case KEY_F9:         printf("KEY_F9");         break;
        case KEY_F10:        printf("KEY_F10");        break;
        case KEY_F11:        printf("KEY_F11");        break;
        case KEY_F12:        printf("KEY_F12");        break;
        //case KEY_F13:        printf("KEY_F13");        break;
        //case KEY_F14:        printf("KEY_F14");        break;
        //case KEY_F15:        printf("KEY_F15");        break;
        //case KEY_F16:        printf("KEY_F16");        break;
        //case KEY_F17:        printf("KEY_F17");        break;
        //case KEY_F18:        printf("KEY_F18");        break;
        //case KEY_F19:        printf("KEY_F19");        break;
        //case KEY_F20:        printf("KEY_F20");        break;
        //case KEY_F21:        printf("KEY_F21");        break;
        //case KEY_F22:        printf("KEY_F22");        break;
        //case KEY_F23:        printf("KEY_F23");        break;
        //case KEY_F24:        printf("KEY_F24");        break;
        default: printf("other keycode: 0x%02X", key);
    }
    printf("\n");
}

void mousedown(Frame* f, int button, int x, int y) {
    (void)f;
    switch (button) {
        case MOUSE_LEFT:   printf("MOUSE_LEFT");   break;
        case MOUSE_RIGHT:  printf("MOUSE_RIGHT");  break;
        case MOUSE_MIDDLE: printf("MOUSE_MIDDLE"); break;
        default: printf("other button: %d", button);
    }
    printf(" at (%d, %d)\n", x, y);
}

int main(int argc, char** argv) {
    argc--, argv++;

    Frame f = {
        .width= 640,
        .height= 480,
        .title= "hi :3",
        .events= {
            .render= render,
            .resize= resize,
            .closing= frame_close,
            .keydown= keydown,
            .mousedown= mousedown,
        },
    };

    if (!frame_create(&f)) return 1;
    text_init();
    frame_loop(&f);
    text_free();
    frame_destroy(&f);

    return 0;
}
