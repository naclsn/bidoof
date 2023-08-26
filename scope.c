#include "scope.h"

bool scope_make(Scope* res, Sym const name) {
    memset(res, 0, sizeof *res);

    res->keepalive.ty = SYM;
    memcpy(&res->keepalive.as.sym, &name, sizeof name);

    return true;
}

void scope_destroy(Scope* self) {
    for (sz k = 0; k < self->count; k++) {
        Obj* it = self->items[k].value;
        if (obj_remdep(&self->keepalive, it) && !it->depnts) {
            obj_destroy(it);
            free(it);
        }
    }

    free(self->items);

    self->items = NULL;
    self->count = 0;
    self->size = 0;
}

sz _binary_search(Scope* self, Sym const name, int* cmp) {
    if (0 == self->count) {
        *cmp = -1;
        return 0;
    }

    sz bel = 0, abo = self->count;
    sz cur, nxt = self->count/2;

    do {
        Sym const key = self->items[cur = nxt].key;

        *cmp = symcmp(name, key);
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

Obj* scope_get(Scope* self, Sym const key) {
    int cmp;
    sz k = _binary_search(self, key, &cmp);
    return 0 == cmp ? self->items[k].value : NULL;
}

Obj* scope_put(Scope* self, Sym const key, Obj* value) {
    {
        sz resize = 0 == self->size ? 16 : self->size;
        if (resize < self->size+1) resize*= 2;

        if (resize != self->size) {
            Entry* niw = reallocarray(self->items, resize, sizeof(Entry));
            if (!niw) return NULL;

            self->items = niw;
            self->size = resize;
        }
    }

    {
        struct Depnt* ka = malloc(sizeof *ka);
        if (!ka) return NULL;

        ka->obj = &self->keepalive;
        ka->next = value->depnts;
        value->depnts = ka;
    }

    int cmp;
    sz k = _binary_search(self, key, &cmp);

    if (0 == cmp) { // replace-at
        Obj* found = self->items[k].value;
        if (obj_remdep(&self->keepalive, found) && !found->depnts) {
            obj_destroy(found);
            free(found);
            found = NULL;
        }

        self->items[k].value = value;

        return found;
    }

    if (0 < cmp) k++; // insert-after
    // else // insert-before

    memmove(self->items+k+1, self->items+k, (self->count-k)*sizeof(Entry));
    memcpy(&self->items[k].key, &key, sizeof key);
    self->items[k].value = value;
    self->count++;

    return NULL;
}
