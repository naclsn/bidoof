#ifndef __BIDOOF_T_JVM__
#define __BIDOOF_T_JVM__

#ifdef BIDOOF_T_IMPLEMENTATION
#define _redef_after_jvm
#undef BIDOOF_IMPLEMENTATION
#undef BIDOOF_T_IMPLEMENTATION
#endif
#include "archives.h"
#ifdef _redef_after_jvm
#undef _redef_after_jvm
#define BIDOOF_IMPLEMENTATION
#define BIDOOF_T_IMPLEMENTATION
#endif

#include "../base.h"
#include "../_common_conf/bipa.h"
#include "../_common_conf/paras.h"

#ifdef BIDOOF_LIST_DEPS
static struct _list_deps_item const _list_deps_me_jvm = {_list_deps_first, "jvm"};
#undef _list_deps_first
#define _list_deps_first &_list_deps_me_jvm
#endif

// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html
paras_make_instrset(jvm_iset,
    paras_make_instr(debug, (0xca), (0xff), (""), (codes[0]))
)

bipa_struct(jvm_class, 1
        , (u8,), _
        )

// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-4.html
adapt_bipa_type(jvm_class)

#ifdef BIDOOF_IMPLEMENTATION


#endif // BIDOOF_IMPLEMENTATION

#endif // __BIDOOF_T_JVM__
