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

bipa_packed(jvm_class_access_flags, u16be, 15
        , 1, acc_public
        , 1, acc_private
        , 1, acc_protected
        , 1, acc_static
        , 1, acc_final
        , 1, acc_synchronized
        , 1, acc_bridge
        , 1, acc_varargs
        , 1, acc_native
        , 1, acc_interface
        , 1, acc_abstract
        , 1, acc_strict
        , 1, acc_synthetic
        , 1, acc_annotation
        , 1, acc_enum
        )

bipa_union(jvm_class_reference_kind, 9
        , (void,), (u8, 1, getField)
        , (void,), (u8, 2, getStatic)
        , (void,), (u8, 3, putField)
        , (void,), (u8, 4, putStatic)
        , (void,), (u8, 5, invokeVirtual)
        , (void,), (u8, 6, invokeStatic)
        , (void,), (u8, 7, invokeSpecial)
        , (void,), (u8, 8, newInvokeSpecial)
        , (void,), (u8, 9, invokeInterface)
        )

bipa_struct(jvm_class_Class,              1, (u16be,), name_index)
bipa_struct(jvm_class_Fieldref,           2, (u16be,), class_index, (u16be,), name_and_type_index)
bipa_struct(jvm_class_Methodref,          2, (u16be,), class_index, (u16be,), name_and_type_index)
bipa_struct(jvm_class_InterfaceMethodref, 2, (u16be,), class_index, (u16be,), name_and_type_index)
bipa_struct(jvm_class_String,             1, (u16be,), string_index)
bipa_struct(jvm_class_Integer,            1, (u32be,), bytes)
bipa_struct(jvm_class_Float,              1, (u32be,), bytes)
bipa_struct(jvm_class_Long,               1, (u64be,), bytes)
bipa_struct(jvm_class_Double,             1, (u64be,), bytes)
bipa_struct(jvm_class_NameAndType,        2, (u16be,), name_index, (u16be,), descriptor_index)
bipa_struct(jvm_class_Utf8,               2, (u16be,), length, (lstr, self->length), bytes)
bipa_struct(jvm_class_MethodHandle,       2, (union, jvm_class_reference_kind), reference_kind, (u16be,), reference_index)
bipa_struct(jvm_class_MethodType,         1, (u16be,), descriptor_index)
bipa_struct(jvm_class_InvokeDynamic,      2, (u16be,), bootstrap_method_attr_index, (u16be,), name_and_type_index)

bipa_union(jvm_class_cp_info, 14
        , (struct, jvm_class_Class),              (u8, 7,  Class)
        , (struct, jvm_class_Fieldref),           (u8, 9,  Fieldref)
        , (struct, jvm_class_Methodref),          (u8, 10, Methodref)
        , (struct, jvm_class_InterfaceMethodref), (u8, 11, InterfaceMethodref)
        , (struct, jvm_class_String),             (u8, 8,  String)
        , (struct, jvm_class_Integer),            (u8, 3,  Integer)
        , (struct, jvm_class_Float),              (u8, 4,  Float)
        , (struct, jvm_class_Long),               (u8, 5,  Long)
        , (struct, jvm_class_Double),             (u8, 6,  Double)
        , (struct, jvm_class_NameAndType),        (u8, 12, NameAndType)
        , (struct, jvm_class_Utf8),               (u8, 1,  Utf8)
        , (struct, jvm_class_MethodHandle),       (u8, 15, MethodHandle)
        , (struct, jvm_class_MethodType),         (u8, 16, MethodType)
        , (struct, jvm_class_InvokeDynamic),      (u8, 18, InvokeDynamic)
        )
bipa_array(jvm_class_cp_infos, (struct, jvm_class_cp_info))

bipa_array(jvm_class_interfaces, (u16be,))

bipa_struct(jvm_class_attribute_info, 3
        , (u16be,), attribute_name_index
        , (u32be,), attribute_length
        , (lstr, self->attribute_length), info
        )
bipa_array(jvm_class_attribute_infos, (struct, jvm_class_attribute_info))

bipa_struct(jvm_class_field_info, 5
        , (packed, jvm_class_access_flags), access_flags
        , (u16be,), name_index
        , (u16be,), descriptor_index
        , (u16be,), attributes_count
        , (array, jvm_class_attribute_infos, k < self->attributes_count), attributes
        )
bipa_array(jvm_class_field_infos, (struct, jvm_class_field_info))

bipa_struct(jvm_class_method_info, 5
        , (packed, jvm_class_access_flags), access_flags
        , (u16be,), name_index
        , (u16be,), descriptor_index
        , (u16be,), attributes_count
        , (array, jvm_class_attribute_infos, k < self->attributes_count), attributes
        )
bipa_array(jvm_class_method_infos, (struct, jvm_class_method_info))

bipa_union(jvm_class_magic_number, 1, (void,), (u32be, 0xcafebabe, _))
bipa_struct(jvm_class, 16
        , (union, jvm_class_magic_number), _
        , (u16be,), minor_version
        , (u16be,), major_version
        , (u16be,), constant_pool_count
        , (array, jvm_class_cp_infos, k+1 < self->constant_pool_count), constant_pool
        , (packed, jvm_class_access_flags), access_flags
        , (u16be,), this_class
        , (u16be,), super_class
        , (u16be,), interfaces_count
        , (array, jvm_class_interfaces, k < self->interfaces_count), interfaces
        , (u16be,), fields_count
        , (array, jvm_class_field_infos, k < self->fields_count), fields
        , (u16be,), methods_count
        , (array, jvm_class_method_infos, k < self->methods_count), methods
        , (u16be,), attributes_count
        , (array, jvm_class_attribute_infos, k < self->attributes_count), attributes
        )

// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-4.html
adapt_bipa_type(jvm_class)

#ifdef BIDOOF_IMPLEMENTATION


#endif // BIDOOF_IMPLEMENTATION

#endif // __BIDOOF_T_JVM__
