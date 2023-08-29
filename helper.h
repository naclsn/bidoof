#include "base.h"

#define typecheck_args_1(__ty1)                                           (__ty1 == res->argv[0]->ty)
#define typecheck_args_2(__ty1, __ty2)                typecheck_args_1(__ty1) && (__ty2 == res->argv[1]->ty)
#define typecheck_args_3(__ty1, __ty2, __ty3)         typecheck_args_2(__ty1, __ty2) && (__ty3 == res->argv[2]->ty)
#define typecheck_args_4(__ty1, __ty2, __ty3, __ty4)  typecheck_args_3(__ty1, __ty2, __ty3) && (__ty4 == res->argv[3]->ty)
#define typecheck_args(__n, ...)                  \
    if (__n == res->argc                          \
            && typecheck_args_##__n(__VA_ARGS__)  \
            ) goto _type_ok

#define typecheck_overloads_1(__tuple1)                                             typecheck_args __tuple1
#define typecheck_overloads_2(__tuple1, __tuple2)            typecheck_overloads_1(__tuple1); typecheck_args __tuple2
#define typecheck_overloads_3(__tuple1, __tuple2, __tuple3)  typecheck_overloads_2(__tuple1, __tuple2); typecheck_args __tuple3

// TODO: .. _meta_##__name = .. or something
#define given_ctor(__ret, __name, __make_also, __n_overloads, ...)  \
    bool _update_##__name(Obj* self);                               \
    bool _make_##__name(Obj* self, Obj* res) {                      \
        (void)self;                                                 \
        res->update = _update_##__name;                             \
        res->ty = __ret;                                            \
        typecheck_overloads_##__n_overloads(__VA_ARGS__);           \
        return false;                                               \
    _type_ok:                                                       \
        return __make_also();                                       \
    }                                                               \
    Obj __name = {.ty= FUN, .as.fun.call= _make_##__name};          \
    bool _update_##__name(Obj* self)

inline bool _make_also_no_op(void) { return true; }
#define simple_ctor(__ret, __name, __n_overloads, ...)  \
    given_ctor(__ret, __name, _make_also_no_op, __n_overloads, __VA_ARGS__)

#define bind_arg_as_Buf buf
#define bind_arg_as_Num num
#define bind_arg_as_Lst lst
#define bind_arg_as_Fun fun
#define bind_arg_as_Sym sym
#define bind_arg(__n, __ty, __name)  \
    __ty const* const __name = &self->argv[__n]->as.bind_arg_as_##__ty

// TODO: .. names_meta[] = {..} or something
#define export_names(...)  \
    char* names[] = {__VA_ARGS__, NULL}
