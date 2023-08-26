#ifndef __BIDOOF_EXTS_H__
#define __BIDOOF_EXTS_H__

#include "base.h"
#include "scope.h"
#include <dlfcn.h>

extern Scope exts_scope;

bool exts_load(char const* filename);
Obj* exts_lookup(char const* name);
void exts_unload(void);

#endif // __BIDOOF_EXTS_H__
