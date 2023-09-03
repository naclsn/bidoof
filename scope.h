#ifndef __BIDOOF_SCOPE_H__
#define __BIDOOF_SCOPE_H__

#include "base.h"

typedef struct ScopeEntry {
    Sym key;
    Obj* value;
} ScopeEntry;

typedef struct Scope {
    ScopeEntry* items;
    sz count;
    sz size;
} Scope;

void scope_clear(Scope* self);
Obj* scope_get(Scope* self, Sym const key);
bool scope_put(Scope* self, Sym const key, Obj* value);
void scope_show(Scope* self);

#endif // __BIDOOF_SCOPE_H__
