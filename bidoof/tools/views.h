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

#ifdef BIDOOF_IMPLEMENTATION

void view_rgba(buf cref pixels, unsigned const width, unsigned const height, char opcref title) {
    exitf("NIY: view_rgba");
    (void)pixels;
    (void)width;
    (void)height;
    (void)title;
}

#endif // BIDOOF_IMPLEMENTATION

#endif // __BIDOOF_T_VIEWS___
