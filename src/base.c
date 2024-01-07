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
            printf("%lf", self->as.flt.val);
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
            printf("%*.s}", indent*3, "");
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
    printf("(%p) keepalive %d", (void*)self, self->keepalive);
    struct Depnt* cur = self->depnts;

    if (!cur)
        printf(", no depnts");
    else do {
        printf("\n%*.s-> ", (curdepth < 0 ? 1 : curdepth+1)*3, "");
        if (0 <= curdepth) obj_show_depnts(cur->obj, curdepth+1);
        cur = cur->next;
    } while (cur);

    if (curdepth < 1) printf("\n");
}

bool obj_call(Obj* self, Obj* res) {
    if (!self->as.fun.call(self, res)) return false;

    for (sz k = 0; k < res->argc; k++) {
        Obj* on = res->argv[k];

        struct Depnt* tail = malloc(sizeof *tail);
        if (!tail) {
            for (; 0 < k; k--) obj_remdep(res, res->argv[k-1]);
            return false;
        }

        tail->obj = res;
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

    if (res->update && !res->update(res)) {
        obj_destroy(res);
        return false;
    }

    return true;
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
        if (obj_remdep(self, dep) && 0 == dep->keepalive) {
            obj_destroy(dep);
            free(dep);
        }
    }

    memset(&self->as, 0, sizeof self->as);
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

#ifdef TRACE_ALLOCS
#undef malloc
#undef free
#undef calloc
#undef realloc
#include <unistd.h>

static void __trace_allocs_puts(char const* s) {
    write(STDERR_FILENO, s, strlen(s));
}
static void __trace_allocs_putp(void* p) {
    char s[19]; s[18] = '\0';
    s[0] = '0'; s[1] = 'x';
    uintptr_t n = (uintptr_t)p;
    for (short k = 7; 0 <= k; k--) {
        char h = n>>4&15, l = n&15;
        s[2+2*k+0] = (9 < h ? 'A'-10 : '0') + h;
        s[2+2*k+1] = (9 < l ? 'A'-10 : '0') + l;
        n>>= 8;
    }
    __trace_allocs_puts(s);
}
static void __trace_allocs_putn(size_t n) {
    char s[21]; s[20] = '\0';
    short k = 20;
    do {
        s[--k] = '0' + n%10;
        n/= 10;
    } while (n);
    while (k) s[--k] = ' ';
    __trace_allocs_puts(s);
}
static void __trace_allocs_putw(size_t r) {
    char s[r+1]; s[r] = '\0';
    while (r) s[--r] = ' ';
    __trace_allocs_puts(s);
}
#define __trace_allocs_putva_1(__t, __v)      __trace_allocs_put##__t(__v)
#define __trace_allocs_putva_2(__t, __v, ...) __trace_allocs_put##__t(__v); __trace_allocs_putva_1(__VA_ARGS__)
#define __trace_allocs_putva_3(__t, __v, ...) __trace_allocs_put##__t(__v); __trace_allocs_putva_2(__VA_ARGS__)
#define __trace_allocs_putva_4(__t, __v, ...) __trace_allocs_put##__t(__v); __trace_allocs_putva_3(__VA_ARGS__)
#define __trace_allocs_putva_5(__t, __v, ...) __trace_allocs_put##__t(__v); __trace_allocs_putva_4(__VA_ARGS__)
#define __trace_allocs_putva_6(__t, __v, ...) __trace_allocs_put##__t(__v); __trace_allocs_putva_5(__VA_ARGS__)
#define __trace_allocs_putva_7(__t, __v, ...) __trace_allocs_put##__t(__v); __trace_allocs_putva_6(__VA_ARGS__)
#define __trace_allocs_putva_8(__t, __v, ...) __trace_allocs_put##__t(__v); __trace_allocs_putva_7(__VA_ARGS__)
#define __trace_allocs_putva_9(__t, __v, ...) __trace_allocs_put##__t(__v); __trace_allocs_putva_8(__VA_ARGS__)
#define __trace_allocs_putva(__n, ...) do { __trace_allocs_putva_##__n(__VA_ARGS__); } while (0)

char const* __trace_hint = NULL;

void* __trace_allocs_malloc(char const* const call, size_t const s) {
    void* const r = malloc(s);
    __trace_allocs_putva(7, s,"malloc ", n,s, s," = ", p,r, w,23, s,__trace_hint, s,call);
    __trace_hint = NULL;
    return r;
}

void  __trace_allocs_free(char const* const call, void* const p) {
    free(p);
    __trace_allocs_putva(5, s,"free ", p,p, w,48, s,__trace_hint, s,call);
    __trace_hint = NULL;
}

void* __trace_allocs_calloc(char const* const call, size_t const c, size_t const s) {
    void* const r = calloc(c, s);
    __trace_allocs_putva(9, s,"calloc ", n,c, s," ", n,s, s," = ", p,r, w,2, s,__trace_hint, s,call);
    __trace_hint = NULL;
    return r;
}

void* __trace_allocs_realloc(char const* const call, void* const p, size_t const s) {
    void* const r = realloc(p, s);
    __trace_allocs_putva(9, s,"realloc ", p,p, s," ", n,s, s," = ", p,r, w,3, s,__trace_hint, s,call);
    __trace_hint = NULL;
    return r;
}
#endif // TRACE_ALLOCS
