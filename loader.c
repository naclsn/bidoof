#include "loader.h"
#include <stdio.h>

void* loaded;

char* ty_str[] = { [BUF]= "BUF", [NUM]= "NUM", [LST]= "LST", [FUN]= "FUN", [SYM]= "SYM" };

bool load_names(char const* filename) {
    void* ext = dlopen(filename, RTLD_LAZY | RTLD_LOCAL);
    if (!ext) return false;

    char** names = dlsym(ext, "names");
    if (!names) {
        dlclose(ext);
        return false;
    }

    puts("names:");
    while (*names) {
        char* it = *names++;
        Obj* at = dlsym(ext, it);
        printf("- %p: (%s) %s\n", (void*)at, ty_str[at->ty], it);
    }

    // TODO: append to global names, keep sorted for binary lookup
    // TODO: append to loaded

    loaded = ext;
    return true;
}

Obj* lookup_name(char const* name) {
    (void)name;
    return NULL;
}

void unload_all(void) {
    dlclose(loaded);

    //for (void* ext = loaded; ext; ext++)
    //    dlclose(ext);
    // TODO: free
}
