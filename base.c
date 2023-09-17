#include "base.h"

void notify_default(char const* s) { printf("INFO: %s\n", s); }
void notify_null(char const* s) { (void)s; }
void (*notify)(char const* s) = notify_default;

void obj_show(Obj const* self, int indent) {
    if (self) printf("(%p) ", (void*)self);

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
            printf("%ld", self->as.num.val);
            break;

        case FLT:
            printf("%Lf", self->as.flt.val);
            break;

        case LST:
            if (indent < 0 || 16 < self->as.lst.len) {
                printf("{ (%zd objects) }", self->as.lst.len);
                break;
            }
            printf("{\n");
            for (sz k = 0; k < self->as.lst.len; k++) {
                printf("%*.s", (indent+1)*3, "");
                obj_show(self->as.lst.ptr[k], indent+1);
                printf(", \n");
            }
            printf("}");
            break;

        case SYM:
            printf(":%s", self->as.sym.txt);
            break;

        case FUN:
            printf("%p", (void*)self->as.fun.call);
            break;

        default:
            printf("(unknown %d)", self->ty);
    }

    if (indent < 1) printf("\n");
}

void obj_show_depnts(Obj const* self, int curdepth) {
    printf("(keepalive %d)", self->keepalive);
    struct Depnt* cur = self->depnts;

    if (!cur)
        printf(" no depnts");
    else do {
        printf("\n%*.s-> %p", (curdepth < 0 ? 1 : curdepth+1)*3, "", (void*)cur->obj);
        if (0 <= curdepth) obj_show_depnts(cur->obj, curdepth+1);
        cur = cur->next;
    } while (cur);

    if (curdepth < 1) printf("\n");
}

Obj* obj_call(Obj* self, u8 argc, Obj** argv) {
    Obj* r = calloc(1, sizeof(Obj) + argc*sizeof(Obj*));
    if (!r) return NULL;

    r->argc = argc;
    memcpy(&r->argv, argv, argc*sizeof(Obj*));

    if (!self->as.fun.call(self, r)) {
        free(r);
        return NULL;
    }

    for (sz k = 0; k < argc; k++) {
        Obj* on = argv[k];

        struct Depnt* tail = malloc(sizeof *tail);
        if (!tail) {
            for (; 0 < k; k--) obj_remdep(r, argv[k-1]);
            free(r);
            return NULL;
        }

        tail->obj = r;
        tail->next = NULL;

        if (!on->depnts)
            on->depnts = tail;
        else {
            struct Depnt* cur = on->depnts;
            while (cur->next) cur = cur->next;
            cur->next = tail;
        }

        on->keepalive++;
    }

    if (!obj_update(r)) {
        obj_destroy(r);
        return NULL;
    }

    return r;
}

bool obj_remdep(Obj* self, Obj* dep) {
    struct Depnt* prev = dep->depnts;
    for (struct Depnt* cur = prev; cur; prev = cur, cur = cur->next) {
        if (self == cur->obj) {
            if (prev == cur)
                dep->depnts = cur->next;
            else
                prev->next = cur->next;
            free(cur);

            dep->keepalive--;
            return true;
        }
    }
    return false;
}

void obj_destroy(Obj* self) {
    bool (*up)(Obj*) = self->update;
    if (up) {
        self->update = NULL;
        up(self);
    }

    // for each deps, remove self from its depnts
    // then delete dep if it has no depnts anymore
    for (sz k = 0; k < self->argc; k++) {
        Obj* dep = self->argv[k];
        if (obj_remdep(self, dep) && 0 == dep->keepalive)
            obj_destroy(dep);
    }

    memset(&self->as, 0, sizeof self->as);
    free(self);
}

static u16 _static_curr_cycle = 0;
bool _update_depnts(Obj* self) {
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
            if (!_update_depnts(it)) return false;
        }
    }

    return true;
}

bool obj_update(Obj* self) {
    _static_curr_cycle++;
    if (self->update && !self->update(self)) return false;
    self->cycle = _static_curr_cycle;
    return _update_depnts(self);
}
