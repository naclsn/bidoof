#include "base.h"

void show2(Obj const* self, int indent) {
    printf("(%p) ", (void*)self);

    if (!self) printf("(null)");

    else switch (self->ty) {
        case BUF:
            printf("[");
            if (16 < self->as.buf.len)
                printf(" (%zd bytes) ", self->as.buf.len);
            else for (sz k = 0; k < self->as.buf.len; k++)
                printf("%s0x%02X", k ? ", " : "", self->as.buf.ptr[k]);
            printf("]");
            break;

        case NUM:
            printf("%d", self->as.num.val);
            break;

        case LST:
            if (indent < 0 || 16 < self->as.lst.len) {
                printf("{ (%zd objects) }", self->as.lst.len);
                break;
            }
            printf("{\n");
            for (sz k = 0; k < self->as.lst.len; k++) {
                printf("%*.s", indent*3, "");
                show2(self->as.lst.ptr+k, indent+1);
                printf(", \n");
            }
            printf("}");
            break;

        case SYM:
            printf(":%.*s", (int)self->as.sym.len, self->as.sym.ptr);
            break;

        case FUN:
            printf("%p", (void*)self->as.fun.call);
            break;

        default:
            printf("(unknown %d)", self->ty);
    }

    if (indent < 1) printf("\n");
}

Obj* show(Obj* self) {
    show2(self, 0);
    return self;
}

void showDepnts(Obj const* self, int depth) {
    struct Depnt* cur = self->depnts;
    while (cur) {
        printf("\n%*.s-> %p", (depth < 0 ? 1 : depth+1)*3, "", (void*)cur->obj);
        if (0 <= depth) showDepnts(cur->obj, depth+1);
        cur = cur->next;
    }
    if (depth < 1) printf("\n");
}

////////////////

Obj* call(Obj* self, u8 argc, Obj** argv) {
    Obj* r = calloc(1, sizeof(Obj) + argc*sizeof(Obj*));
    if (!r) return r;

    r->argc = argc;
    memcpy(&r->argv, argv, argc*sizeof(Obj*));

    printf("coucou %p\n", (void*)self);
    if (!self->as.fun.call(self, r)) {
        free(r);
        return NULL;
    }

    for (sz k = 0; k < argc; k++) {
        Obj* on = argv[k];

        struct Depnt* tail = malloc(sizeof *tail);
        if (!tail) exit(1);

        tail->obj = r;
        tail->next = NULL;

        if (!on->depnts)
            on->depnts = tail;
        else {
            struct Depnt* cur = on->depnts;
            while (cur->next) cur = cur->next;
            cur->next = tail;
        }
    }

    return r;
}

void destroy(Obj* self) {
    bool (*up)(Obj*) = self->update;
    self->update = NULL;
    up(self);

    // for each deps, remove self from depnt
    // then delete dep if it has no depnt anymore
    for (sz k = 0; k < self->argc; k++) {
        Obj* dep = self->argv[k];
        struct Depnt* prev = dep->depnts;
        for (struct Depnt* cur = prev; cur; prev = cur, cur = cur->next) {
            if (self == cur->obj) {
                if (prev != cur)
                    dep->depnts = cur->next;
                else
                    prev->next = cur->next;

                cur->next = NULL;
                cur->obj = NULL;
                free(cur);

                if (!dep->depnts) destroy(dep);
                break;
            }
        }
    }

    memset(&self->as, 0, sizeof self->as);
    free(self);
}

static u16 _static_curr_cycle = 0;
bool _priv_update_depnts(Obj* self) {
    u32 needs_up = 0;
    u16 count = 0;

    for (struct Depnt* cur = self->depnts; cur; cur = cur->next) {
        Obj* it = cur->obj;
        count++;
        needs_up<<= 1;
        if (_static_curr_cycle != it->cycle) {
            if (it->update && !it->update(it)) return false;
            it->cycle = _static_curr_cycle;
            needs_up|= 1;
        }
    }

    for (struct Depnt* cur = self->depnts; cur; cur = cur->next) {
        Obj* it = cur->obj;
        if ((1 << --count) & needs_up) {
            if (!_priv_update_depnts(it)) return false;
        }
    }

    return true;
}

bool update(Obj* self) {
    _static_curr_cycle++;
    if (self->update && !self->update(self)) return false;
    self->cycle = _static_curr_cycle;
    return _priv_update_depnts(self);
}
