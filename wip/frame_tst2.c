#include <stdio.h>

#define FRAME_IMPLEMENTATION
#include "../ext/views/frame.h"
#define TEXT_IMPLEMENTATION
#include "../ext/views/text.h"
//#include <GL/glext.h>

void render1(Frame* f) {
    glClearColor(.12f, .13f, .14f, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    {
        GLsizei width = 8;
        GLsizei height = 16;
        float const data[8*16] = {
            0,1,1,1,1,0,0,0,
            0,1,1,1,1,1,1,0,
            0,1,1,0,0,1,1,0,
            0,1,1,0,0,1,1,0,
            0,1,1,1,1,1,0,0,
            0,1,1,0,0,1,0,0,
            0,1,1,1,1,1,0,0,
            0,1,1,1,1,0,0,0,
            0,1,1,0,0,1,1,0,
            0,1,1,0,0,1,1,0,
            0,1,1,0,0,1,1,0,
            0,1,1,1,1,1,1,0,
            0,1,1,0,0,1,1,0,
            0,1,1,0,0,1,1,0,
            0,0,1,1,1,1,0,0,
            0,0,0,1,1,0,0,0,
        };

        glTexImage2D(GL_TEXTURE_2D,
                0, GL_RED,
                width, height,
                0, GL_RED,
                GL_FLOAT,
                data);

        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glBegin(GL_QUADS);
        for (int i = 0; i < f->width; i++) for (int j = 0; j < f->height; j++) {
            float scale = 8;
            glOrtho(0, width, 0, height, -1, 1);
            glTexCoord2f(0, 0); glVertex2f((float)i/f->width*2-1, (float)j/f->height*2-1);
            glTexCoord2f(0, 1); glVertex2f(0, scale*height/f->height);
            glTexCoord2f(1, 1); glVertex2f(scale*width/f->width, scale*height/f->height);
            glTexCoord2f(1, 0); glVertex2f(scale*width/f->width, 0);
        }
        glEnd();
    }
    glDeleteTextures(1, &tex);
    glDisable(GL_TEXTURE_2D);
}

void render(Frame* f) {
    (void)f;

    glClearColor(.12f, .13f, .14f, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    text_allf8x8();

    //char const* const txt = "hi :<";
    //text_draw(txt, strlen(txt), 0, 0, 3.f);
}

void resize(Frame* f, int w, int h) {
    (void)f;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, w/10, 0.0, h/10, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyup(Frame* f, char key) {
    if (27 == key) frame_close(f);
}

int main(int argc, char** argv) {
    argc--, *argv++;

    Frame f = {
        .width= 640,
        .height= 480,
        .title= "hi :3",
        .events= {
            .render= render,
            .resize= resize,
            .closing= frame_close,
            .keyup= keyup,
        },
    };

    if (!frame_create(&f)) return 1;
    text_init();
    frame_loop(&f);
    text_free();
    frame_destroy(&f);

    return 0;
}
