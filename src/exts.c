#if (defined(__MINGW32__) || defined(__MINGW64__)) && !defined(__CYGWIN__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define RTLD_LOCAL 0
#define RTLD_LAZY 1
void* dlopen(char const* file, int flags) {
    (void)flags;
    // << be sure to use backslashes (\), not forward slashes (/) >>
    sz len = strlen(file)+1;
    char* bs = alloca(len);
    for (sz k = 0; k < len+1; k++)
        bs[k] = '/' == file[k] ? '\\' : file[k];
    return LoadLibrary(bs);
}
void* dlsym(void* handle, char const* name) {
    return GetProcAddress(handle, name);
}
void dlclose(void* handle) {
    FreeLibrary(handle);
}
char const* dlerror(void) {
    static char buf[256];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            buf, sizeof buf / sizeof buf[0], NULL);
    return buf;
}
#else
#include <dlfcn.h>
#endif

#include "exts.h"

dyarr(void*) loaded = {0};
Scope exts_scope = {0};

bool exts_load(char const* filename) {
    void* ext = dlopen(filename, RTLD_LAZY | RTLD_LOCAL);
    if (!ext) {
        notify(dlerror());
        return false;
    }

    char** names = dlsym(ext, "names");
    if (!names) {
        notify("no 'names' in lib");
        dlclose(ext);
        return false;
    }

    // append to loaded (to-be-dlclose-ed)
    {
        void** niw = dyarr_push(&loaded);
        if (!niw) {
            notify("OOM");
            dlclose(ext);
            return false;
        }
        *niw = ext;
    }

    for (; *names; names++) {
        Meta* meta = dlsym(ext, *names);

        if (meta) {
            Sym const key = mksym(meta->name);
            Obj* value = &meta->obj;

            // FIXME: expectedly crashes when putting at a key that already
            // exists (it tries to delete/free the existing value)
            if (!scope_put(&exts_scope, key, value)) {
                notify("OOM");
                return false;
            }

        } else {
            char m[32];
            sprintf(m, "no meta for '%s'\n", *names);
            notify(m);
        }
    }

    return true;
}

Meta* exts_lookup(Sym const name) {
    Obj* obj = scope_get(&exts_scope, name);
    return obj ? frommember(obj, Meta, obj) : NULL;
}

void exts_unload(void) {
    // don't destroy the Obj, as they where never created (these are static)
    //scope_destroy(&exts_scope);
    free(exts_scope.items);

    for (sz k = 0; k < loaded.len; k++)
        dlclose(loaded.ptr[k]);
    dyarr_clear(&loaded);
}
