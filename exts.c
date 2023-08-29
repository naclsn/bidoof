#include <stddef.h>
#include "exts.h"

struct LoadedList {
    void* ext;
    struct LoadedList* next;
}* loaded = NULL;

Scope exts_scope = {0};

bool exts_load(char const* filename) {
    void* ext = dlopen(filename, RTLD_LAZY | RTLD_LOCAL);
    if (!ext) {
        puts(dlerror());
        return false;
    }

    char** names = dlsym(ext, "names");
    if (!names) {
        puts("no 'names' in lib");
        dlclose(ext);
        return false;
    }

    // append to loaded (to-be-dlclose-ed)
    {
        struct LoadedList* niw = malloc(sizeof *niw);
        if (!niw) exit(1);

        niw->ext = ext;
        niw->next = NULL;

        if (!loaded)
            loaded = niw;
        else {
            struct LoadedList* cur = loaded;
            while (cur->next) cur = cur->next;
            cur->next = niw;
        }
    }

    for (; *names; names++) {
        Meta* meta = dlsym(ext, *names);
        if (meta) {
            Sym const key = {meta->name, strlen(meta->name)};
            Obj* value = &meta->obj;
            scope_put(&exts_scope, key, value);
        } else printf("no meta for '%s'\n", *names);
    }

    return true;
}

Meta* exts_lookup(Sym const name) {
    Obj* obj = scope_get(&exts_scope, name);
    Meta* meta = (Meta*)( ((char*)obj) - offsetof(struct Meta, obj) );
    return meta;
}

void exts_unload(void) {
    // don't destroy the Obj, as they where never created (these are static)
    //scope_destroy(&exts_scope);
    free(exts_scope.items);

    for (struct LoadedList* it = loaded, * next; it; it = next) {
        dlclose(it->ext);
        next = it->next;
        free(it);
    }
}
