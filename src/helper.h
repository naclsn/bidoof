#include "base.h"

#define _link_name_par_1(__ty1, __nm1)                                            __ty1
#define _link_name_par_2(__ty1, __nm1, __ty2, __nm2)                              __ty1##_##__ty2
#define _link_name_par_3(__ty1, __nm1, __ty2, __nm2, __ty3, __nm3)                __ty1##_##__ty2##_##__ty3
#define _link_name_par_4(__ty1, __nm1, __ty2, __nm2, __ty3, __nm3, __ty4, __nm4)  __ty1##_##__ty2##_##__ty3##_##__ty4

#define _link_name2(__ret, __name, __lk_par_name)  \
    _update_##__ret##_##__name##_##__lk_par_name
#define _link_name1(__ret, __name, __lk_par_name)  \
    _link_name2(__ret, __name, __lk_par_name)
#define link_name(__name, __n_par, __ret, __ufname, ...)  \
    _link_name1(__ret, __name, _link_name_par_##__n_par(__VA_ARGS__))



#define ty_for_ANY Obj
#define ty_for_BUF Buf
#define ty_for_NUM Num
#define ty_for_FLT Flt
#define ty_for_LST Lst
#define ty_for_FUN Fun
#define ty_for_SYM Sym

#define _link_uf_pars(__k, __n, __inv, __ty, __nm)  , ty_for_##__ty const* const __nm
#define link_uf_pars(__n_par, __ret, __ufname, ...)  \
    ty_for_##__ret* self _FOR_TYNM(__n_par, _link_uf_pars, 0, __VA_ARGS__)

#define as_for_ANY(__o) (__o)
#define as_for_BUF(__o) (&(__o)->as.buf)
#define as_for_NUM(__o) (&(__o)->as.num)
#define as_for_FLT(__o) (&(__o)->as.flt)
#define as_for_LST(__o) (&(__o)->as.lst)
#define as_for_FUN(__o) (&(__o)->as.fun)
#define as_for_SYM(__o) (&(__o)->as.sym)

#define _link_uf_args(__k, __n, __inv, __ty, __nm)  , as_for_##__ty(self->args.ptr[__k])
#define link_uf_args(__n_par, __ret, __ufname, ...)  \
    as_for_##__ret(self) _FOR_TYNM(__n_par, _link_uf_args, 0, __VA_ARGS__)



#define _ufname(__n_par, __ret, __ufname, ...)  __ufname

#define link_overloads_1(__name, __par1)                                \
    static bool _ufname __par1 (link_uf_pars __par1);                   \
    static bool _CALL(link_name, __name, _UNPACK __par1) (Obj* self) {  \
        return _ufname __par1 (link_uf_args __par1);                    \
    }
#define link_overloads_2(__name, __par1, __par2)          link_overloads_1(__name, __par1) link_overloads_1(__name, __par2)
#define link_overloads_3(__name, __par1, __par2, __par3)  link_overloads_1(__name, __par1) link_overloads_2(__name, __par2, __par3)



#define _typecheck_args(__k, __n, __inv, __ty, __nm)  && (__ty == ANY || __ty == res->args.ptr[__k]->ty)
#define typecheck_args(__name, __n_par, __ret, __ufname, ...)                            \
    if (__n_par == res->args.len _FOR_TYNM(__n_par, _typecheck_args, 0, __VA_ARGS__)) {  \
        res->update = link_name(__name, __n_par, __ret, __ufname, __VA_ARGS__);          \
        res->ty = __ret;                                                                 \
    }

#define typecheck_overloads_1(__name, __par1)                                                     _CALL(typecheck_args, __name, _UNPACK __par1)
#define typecheck_overloads_2(__name, __par1, __par2)          typecheck_overloads_1(__name, __par1) else _CALL(typecheck_args, __name, _UNPACK __par2)
#define typecheck_overloads_3(__name, __par1, __par2, __par3)  typecheck_overloads_2(__name, __par1, __par2) else _CALL(typecheck_args, __name, _UNPACK __par3)



#define _document_pars(__k, __n, __inv, __ty, __nm)  {.ty= __ty, .name= #__nm},
#define document_pars(__n_par, __ret, __ufname, ...) {          \
        .ret= __ret,                                            \
        .params= (struct MetaOvlPrm[]){                         \
            _FOR_TYNM(__n_par, _document_pars, 0, __VA_ARGS__)  \
            {0}                                                 \
        }                                                       \
    }

#define document_overloads_1(__par1)                                        _CALL(document_pars, _UNPACK __par1)
#define document_overloads_2(__par1, __par2)          document_overloads_1(__par1), _CALL(document_pars, _UNPACK __par2)
#define document_overloads_3(__par1, __par2, __par3)  document_overloads_2(__par1, __par2), _CALL(document_pars, _UNPACK __par3)



static inline bool _no_make_also(Obj* fun, Obj* res) {
    (void)fun;
    (void)res;
    return true;
}



#define fail(__msg) do {      \
        notify(__msg);        \
        return false;         \
    } while (true)

#define failf(__sz, __msg, ...) do {              \
        notify_printf(__sz, __msg, __VA_ARGS__);  \
        return false;                             \
    } while (true)

#define ctor_given(__name, __doc, __make, __meta_overloads)  \
    Meta __name = {                                          \
        .doc= __doc,                                         \
        .name= #__name,                                      \
        .overloads= __meta_overloads,                        \
        .obj= {.ty= FUN, .as.fun.call= __make},              \
    }

#define ctor_w_also(__n_overloads, __name, __make_also, __doc, ...)  \
    static bool __make_also(Obj* fun, Obj* res);                     \
    link_overloads_##__n_overloads(__name, __VA_ARGS__)              \
    static bool _make_##__name(Obj* self, Obj* res) {                \
        (void)self;                                                  \
        (void)_no_make_also;                                         \
        typecheck_overloads_##__n_overloads(__name, __VA_ARGS__)     \
        else fail("no such overload for function " #__name);         \
        return __make_also(self, res);                               \
    }                                                                \
    ctor_given(__name, __doc, _make_##__name, ((struct MetaOvl[]){   \
        document_overloads_##__n_overloads(__VA_ARGS__),             \
        {0}                                                          \
    }))

#define ctor_simple(__n_overloads, __name, __doc, ...)  \
    ctor_w_also(__n_overloads, __name, _no_make_also, __doc, __VA_ARGS__)

// XXX: does it need the `__declspec(dllexport)`, here and at the `Meta`s?
#define export_names(...)  \
    char const* const names[] = {__VA_ARGS__, NULL}



#define destroyed(__self) (!frommember(__self, Obj, as)->update)
#define getdata(__self) (frommember(__self, Obj, as)->data)

#define inline_call_assign(__name, __f, __argc, __argv)      \
    {                                                        \
        Obj __name = {.args= {.len= __argc, .ptr= __argv}};  \
                                                             \
        Obj* _f = __f;                                       \
        bool (*_res_up)(Obj*) = NULL;                        \
        if (_f->as.fun.call(_f, &__name)                     \
                && (!(_res_up = __name.update)               \
                    || _res_up(&__name)) ) {

#define inline_call_failed(__name)                           \
        } else {

#define inline_call_cleanup(__name)                          \
        }                                                    \
        __name.update = NULL;                                \
        if (_res_up) _res_up(&__name);                       \
    }

#define enum_symcvt(__enum_name, __var_name, __n, __sym, ...)       \
    enum __enum_name {__VA_ARGS__} __var_name                       \
            = symcvt(__n, __sym, __VA_ARGS__);
#define try_enum_symcvt(__enum_name, __var_name, __n, __sym, ...)   \
    enum_symcvt(__enum_name, __var_name, __n, __sym, __VA_ARGS__);  \
    if ((enum __enum_name)-1 == __var_name)                         \
        fail("expected one of: " #__VA_ARGS__);
