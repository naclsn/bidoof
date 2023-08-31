#include "base.h"

#define _UNPACK(...) __VA_ARGS__
#define _CALL(__macro, ...) __macro(__VA_ARGS__)

#define _FOR_TYNM_1(__macro, __ty1, __nm1)                                                                 __macro(0, __ty1, __nm1)
#define _FOR_TYNM_2(__macro, __ty1, __nm1, __ty2, __nm2)                              _FOR_TYNM_1(__macro, __ty1, __nm1) __macro(1, __ty2, __nm2)
#define _FOR_TYNM_3(__macro, __ty1, __nm1, __ty2, __nm2, __ty3, __nm3)                _FOR_TYNM_2(__macro, __ty1, __nm1, __ty2, __nm2) __macro(2, __ty3, __nm3)
#define _FOR_TYNM_4(__macro, __ty1, __nm1, __ty2, __nm2, __ty3, __nm3, __ty4, __nm4)  _FOR_TYNM_3(__macro, __ty1, __nm1, __ty2, __nm2, __ty3, __nm3) __macro(3, __ty4, __nm4)
#define _FOR_TYNM(__n, __macro, ...)  _FOR_TYNM_##__n(__macro, __VA_ARGS__)



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



#define ty_for_BUF Buf
#define ty_for_NUM Num
#define ty_for_LST Lst
#define ty_for_FUN Fun
#define ty_for_SYM Sym

#define _link_uf_pars(__k, __ty, __nm)  , ty_for_##__ty const* const __nm
#define link_uf_pars(__n_par, __ret, __ufname, ...)  \
    Obj* self, ty_for_##__ret* r _FOR_TYNM_##__n_par(_link_uf_pars, __VA_ARGS__)

#define as_for_BUF buf
#define as_for_NUM num
#define as_for_LST lst
#define as_for_FUN fun
#define as_for_SYM sym

#define _link_uf_args(__k, __ty, __nm)  , &self->argv[__k]->as.as_for_##__ty
#define link_uf_args(__n_par, __ret, __ufname, ...)  \
    self, &self->as.as_for_##__ret _FOR_TYNM_##__n_par(_link_uf_args, __VA_ARGS__)



#define _ufname(__n_par, __ret, __ufname, ...)  __ufname

#define link_overloads_1(__name, __par1)  \
    inline bool _ufname __par1 (link_uf_pars __par1);  \
    bool _CALL(link_name, __name, _UNPACK __par1) (Obj* self) {  \
        return _ufname __par1 (link_uf_args __par1);  \
    }
#define link_overloads_2(__name, __par1, __par2)          link_overloads_1(__name, __par1) link_overloads_1(__name, __par2)
#define link_overloads_3(__name, __par1, __par2, __par3)  link_overloads_1(__name, __par1) link_overloads_2(__name, __par2, __par3)



#define _typecheck_args(__k, __ty, __nm)  && (__ty == res->argv[__k]->ty)
#define typecheck_args(__name, __n_par, __ret, __ufname, ...)                      \
    if (__n_par == res->argc _FOR_TYNM_##__n_par(_typecheck_args, __VA_ARGS__)) {  \
        res->update = link_name(__name, __n_par, __ret, __ufname, __VA_ARGS__);    \
        res->ty = __ret;                                                           \
    }

#define typecheck_overloads_1(__name, __par1)                                                     _CALL(typecheck_args, __name, _UNPACK __par1)
#define typecheck_overloads_2(__name, __par1, __par2)          typecheck_overloads_1(__name, __par1) else _CALL(typecheck_args, __name, _UNPACK __par2)
#define typecheck_overloads_3(__name, __par1, __par2, __par3)  typecheck_overloads_2(__name, __par1, __par2) else _CALL(typecheck_args, __name, _UNPACK __par3)

#define _document_pars(__k, __ty, __nm)  {.ty= __ty, .name= #__nm},
#define document_pars(__n_par, __ret, __ufname, ...) {        \
        .ret= __ret,                                          \
        .params= (struct MetaOvlPrm[]){                       \
            _FOR_TYNM_##__n_par(_document_pars, __VA_ARGS__)  \
            {0}                                               \
        }                                                     \
    }

#define document_overloads_1(__par1)                                        _CALL(document_pars, _UNPACK __par1)
#define document_overloads_2(__par1, __par2)          document_overloads_1(__par1), _CALL(document_pars, _UNPACK __par2)
#define document_overloads_3(__par1, __par2, __par3)  document_overloads_2(__par1, __par2), _CALL(document_pars, _UNPACK __par3)


#define ctor_given(__name, __doc, __make, __meta_overloads)  \
    Meta __name = {                                          \
        .doc= __doc,                                         \
        .name= #__name,                                      \
        .overloads= __meta_overloads,                        \
        .obj= {.ty= FUN, .as.fun.call= __make},              \
    }

#define ctor_w_also(__n_overloads, __name, __make_also, __doc, ...)  \
    inline bool __make_also(Obj* fun, Obj* res);                     \
    link_overloads_##__n_overloads(__name, __VA_ARGS__)              \
    bool _make_##__name(Obj* self, Obj* res) {                       \
        (void)self;                                                  \
        typecheck_overloads_##__n_overloads(__name, __VA_ARGS__)     \
        else return false;                                           \
        return __make_also(self, res);                               \
    }                                                                \
    ctor_given(__name, __doc, _make_##__name, ((struct MetaOvl[]){   \
        document_overloads_##__n_overloads(__VA_ARGS__),             \
        {0}                                                          \
    }))

#define ctor_simple(__n_overloads, __name, __doc, ...)  \
    ctor_w_also(__n_overloads, __name, _no_make_also, __doc, __VA_ARGS__)

bool _no_make_also(Obj* fun, Obj* res) {
    (void)fun;
    (void)res;
    return true;
}



#define export_names(...)  \
    char* names[] = {__VA_ARGS__, NULL}
