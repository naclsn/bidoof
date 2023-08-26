#include "loader.h"

struct LoadedList {
    void* ext;
    struct LoadedList* next;
}* loaded = NULL;

struct GlobalList globals = {0};

sz binary_search_globals(Sym const name, int* cmp) {
    if (0 == globals.count) {
        *cmp = -1;
        return 0;
    }

    sz bel = 0, abo = globals.count;
    sz cur, nxt = globals.count/2;

    do {
        cur = nxt;

        Sym const key = globals.items[cur].key;
        //printf("  now at [%zu] \"%s\" %zu\n", cur, key.ptr,
        //        name.len < key.len ? name.len : key.len);

        *cmp = memcmp(name.ptr, key.ptr,
                name.len < key.len ? name.len : key.len);
        if (0 == *cmp) *cmp = name.len - key.len;

        if (0 == *cmp) return cur;

        else if (*cmp < 0)
            abo = cur;
        else
            bel = cur;

        nxt = (bel+abo)/2;
    } while (cur != nxt);

    return cur;
}

bool load_names(char const* filename) {
    void* ext = dlopen(filename, RTLD_LAZY | RTLD_LOCAL);
    if (!ext) {
        //puts(dlerror());
        return false;
    }

    char** names = dlsym(ext, "names");
    if (!names) {
        //puts("no 'names' in lib");
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

    // append to global names, keep sorted for binary lookup
    {
        sz count = 0;
        for (char** it = names; *it; it++, count++);

        sz resize = 0 == globals.size ? 1 : globals.size;
        while (resize < globals.size+count) resize*= 2;
        globals.size = resize;
        globals.items = realloc(globals.items, resize*sizeof(struct Entry));
        if (!globals.items) exit(1);

        for (; *names; names++) {
            Sym const key = {*names, strlen(*names)};
            Obj* value = dlsym(ext, key.ptr);

            int cmp;
            sz k = binary_search_globals(key, &cmp);

            if (0 == cmp) {
                //printf("name already exists!\n");
                continue;
            }

            if (0 < cmp) k++; // insert-after

            memmove(globals.items+k+1, globals.items+k, (globals.count-k)*sizeof(struct Entry));
            memcpy(&globals.items[k].key, &key, sizeof key);
            globals.items[k].value = value;
            globals.count++;
        }
    }

    return true;
}

Obj* lookup_name(char const* name) {
    Sym const key = {.ptr= name, .len= strlen(name)};

    int cmp;
    sz k = binary_search_globals(key, &cmp);

    return 0 == cmp ? globals.items[k].value : NULL;
}

void unload_all(void) {
    for (struct LoadedList* it = loaded, * next; it; it = next) {
        dlclose(it->ext);
        next = it->next;
        free(it);
    }

    free(globals.items);
}
