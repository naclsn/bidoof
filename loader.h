#ifndef __BIDOOF_LOADER_H__
#define __BIDOOF_LOADER_H__

#include "base.h"
#include <dlfcn.h>

bool load_names(char const* filename);
Obj* lookup_name(char const* name);
void unload_all(void);

#endif // __BIDOOF_LOADER_H__
