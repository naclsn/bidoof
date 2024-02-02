#ifndef __BIDOOF_T_VIEWS___
#define __BIDOOF_T_VIEWS___

#include "../base.h"
#ifdef BIDOOF_IMPLEMENTATION
#define FRAME_IMPLEMENTATION
#include "../utils/frame.h"
#endif

void view_rgba(buf cref pixels, unsigned const width, unsigned const height, char opref title);

#ifdef BIDOOF_IMPLEMENTATION

void view_rgba(buf cref pixels, unsigned const width, unsigned const height, char opref title) {
    exitf("NIY: view_rgba");
    (void)pixels;
    (void)width;
    (void)height;
    (void)title;
}

#endif // BIDOOF_IMPLEMENTATION

#endif // __BIDOOF_T_VIEWS___
