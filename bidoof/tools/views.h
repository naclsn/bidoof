#ifndef __BIDOOF_T_VIEWS___
#define __BIDOOF_T_VIEWS___

#include "../base.h"
#ifdef BIDOOF_IMPLEMENTATION
#define FRAME_IMPLEMENTATION
#include "../utils/frame.h"
#endif

#ifdef BIDOOF_LIST_DEPS
static struct _list_deps_item const _list_deps_me_views = {_list_deps_first, "views"};
#undef _list_deps_first
#define _list_deps_first &_list_deps_me_views
#endif

void view_rgba(buf cref pixels, unsigned const width, unsigned const height, char opcref title);
void view_rgb(buf cref pixels, unsigned const width, unsigned const height, char opcref title);

#ifdef BIDOOF_IMPLEMENTATION

static void _view_render_texture(Frame ref self) {
    GLuint tex = (GLuint)(sz)self->userdata;

    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(-1, -1);
        glTexCoord2f(1, 0); glVertex2f( 1, -1);
        glTexCoord2f(1, 1); glVertex2f( 1,  1);
        glTexCoord2f(0, 1); glVertex2f(-1,  1);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

static void _view_makeframe_texture(buf cref pixels, unsigned const width, unsigned const height, char cref title, GLint gl_internalFormat, GLenum gl_format) {
    unsigned const small = width < height ? width : height;
    float const scale = small < 256 ? 256./small : 1;
    Frame frame = {
        .width= width*scale,
        .height= height*scale,
        .title= title,
        .events= {
            .render= _view_render_texture,
            .closing= frame_close,
        },
    };

    if (!frame_create(&frame)) exitf("could not create frame for view");

    GLuint tex;
    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, gl_internalFormat, width, height, 0, gl_format, GL_UNSIGNED_BYTE, pixels->ptr);
    frame.userdata = (void*)(sz)tex;
    glBindTexture(GL_TEXTURE_2D, 0);

    frame_loop(&frame);
    frame_destroy(&frame);
}

void view_rgba(buf cref pixels, unsigned const width, unsigned const height, char opcref title) {
    if (pixels->len < width*height*4) exitf("not enough data for view");
    _view_makeframe_texture(pixels, width, height, title ? title : "Untitled RGBA view", GL_RGBA8, GL_RGBA);
}

void view_rgb(buf cref pixels, unsigned const width, unsigned const height, char opcref title) {
    if (pixels->len < width*height*3) exitf("not enough data for view");
    _view_makeframe_texture(pixels, width, height, title ? title : "Untitled RGB view", GL_RGB8, GL_RGB);
}

#endif // BIDOOF_IMPLEMENTATION

#endif // __BIDOOF_T_VIEWS___
