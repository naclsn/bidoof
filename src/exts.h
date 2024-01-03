#ifndef __BIDOOF_EXTS_H__
#define __BIDOOF_EXTS_H__

#include "scope.h"

extern Scope exts_scope;

bool exts_load(char const* filename);
Meta* exts_lookup(Sym const name);
void exts_unload(void);

#endif // __BIDOOF_EXTS_H__
