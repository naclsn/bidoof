#include "scope.h"

void scope_clear(Scope* self) {
    for (sz k = 0; k < self->count; k++) {
        Obj* it = self->items[k].value;
        if (0 == --it->keepalive) obj_destroy(it);
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

    value->keepalive++;

    int cmp;
    sz k = _binary_search(self, key, &cmp);

    if (0 == cmp) { // replace-at
        Obj* found = self->items[k].value;
        self->items[k].value = value;

        if (0 == --found->keepalive) {
            obj_destroy(found);
            return NULL;
        }
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
