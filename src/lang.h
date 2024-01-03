#ifndef __BIDOOF_LANG_H__
#define __BIDOOF_LANG_H__

#include "scope.h"

bool lang_process(char const* name, char const* script, Scope* scope);
void lang_show_tokens(char const* name, char const* script);

#endif // __BIDOOF_LANG_H__
