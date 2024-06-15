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

// paras(jvm_bytecode) {{{
#define _op_u16_f(k) (u16)(bytes[k]<<8 | bytes[k+1])
#define _op_u16_e(k) args[k].u>>8 & 0xff, args[k].u & 0xff
#define _op_i16_f(k) (i16)(bytes[k]<<8 | bytes[k+1])
#define _op_i16_e(k) args[k].i>>8 & 0xff, args[k].i & 0xff
#define _op_u32_f(k) (u32)(bytes[k]<<24 | bytes[k+1]<<16 | bytes[k+2]<<8 | bytes[k+3])
#define _op_u32_e(k) args[k].u>>24 & 0xff, args[k].u>>16 & 0xff, args[k].u>>8 & 0xff, args[k].u & 0xff
#define _op_i32_f(k) (i32)(bytes[k]<<24 | bytes[k+1]<<16 | bytes[k+2]<<8 | bytes[k+3])
#define _op_i32_e(k) args[k].i>>24 & 0xff, args[k].i>>16 & 0xff, args[k].i>>8 & 0xff, args[k].i & 0xff
#define _arr_ty_f(k) bytes[k] < 4 || 11 < bytes[k] ? "???" : ((char*[]){"boolean", "char", "float", "double", "byte", "short", "int", "long"})[bytes[k]-4]
#define _arr_ty_e(k) ( !memcmp("boolean", args[k].s, 7) ?  4  \
                     : !memcmp("char",    args[k].s, 4) ?  5  \
                     : !memcmp("float",   args[k].s, 5) ?  6  \
                     : !memcmp("double",  args[k].s, 6) ?  7  \
                     : !memcmp("byte",    args[k].s, 4) ?  8  \
                     : !memcmp("short",   args[k].s, 5) ?  9  \
                     : !memcmp("int",     args[k].s, 3) ? 10  \
                     : !memcmp("long",    args[k].s, 4) ? 11  \
                     : paras_mark_invalid())
bool _jvm_tableswitch_f(buf cref src, sz ref at, buf ref res);
bool _jvm_tableswitch_e(buf cref line, buf ref res);
bool _jvm_lookupswitch_f(buf cref src, sz ref at, buf ref res);
bool _jvm_lookupswitch_e(buf cref line, buf ref res);

// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-6.html
paras_make_instrset(jvm_bytecode,
    paras_make_instr(nop,             (0x00),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(aconst_null,     (0x01),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(iconst_m1,       (0x02),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(iconst_0,        (0x03),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(iconst_1,        (0x04),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(iconst_2,        (0x05),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(iconst_3,        (0x06),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(iconst_4,        (0x07),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(iconst_5,        (0x08),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lconst_0,        (0x09),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lconst_1,        (0x0a),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fconst_0,        (0x0b),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fconst_1,        (0x0c),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fconst_2,        (0x0d),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dconst_0,        (0x0e),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dconst_1,        (0x0f),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(bipush,          (0x10, 0),          (0xff, 0),                ("%i", bytes[1]),                    (codes[0], args[0].i))
    paras_make_instr(sipush,          (0x11, 0, 0),       (0xff, 0, 0),             ("%i", _op_i16_f(1)),                (codes[0], _op_i16_e(0)))
    paras_make_instr(ldc,             (0x12, 0),          (0xff, 0),                ("#%u", bytes[1]),                   (codes[0], args[0].u))
    paras_make_instr(ldc_w,           (0x13, 0, 0),       (0xff, 0, 0),             ("#%u", _op_u16_f(1)),               (codes[0], _op_u16_e(0)))
    paras_make_instr(ldc2_w,          (0x14, 0, 0),       (0xff, 0, 0),             ("#%u", _op_u16_f(1)),               (codes[0], _op_u16_e(0)))
    paras_make_instr(iload,           (0x15, 0),          (0xff, 0),                ("_%u", bytes[1]),                   (codes[0], args[0].u))
    paras_make_instr(lload,           (0x16, 0),          (0xff, 0),                ("_%u", bytes[1]),                   (codes[0], args[0].u))
    paras_make_instr(fload,           (0x17, 0),          (0xff, 0),                ("_%u", bytes[1]),                   (codes[0], args[0].u))
    paras_make_instr(dload,           (0x18, 0),          (0xff, 0),                ("_%u", bytes[1]),                   (codes[0], args[0].u))
    paras_make_instr(aload,           (0x19, 0),          (0xff, 0),                ("_%u", bytes[1]),                   (codes[0], args[0].u))
    paras_make_instr(iload_0,         (0x1a),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(iload_1,         (0x1b),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(iload_2,         (0x1c),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(iload_3,         (0x1d),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lload_0,         (0x1e),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lload_1,         (0x1f),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lload_2,         (0x20),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lload_3,         (0x21),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fload_0,         (0x22),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fload_1,         (0x23),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fload_2,         (0x24),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fload_3,         (0x25),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dload_0,         (0x26),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dload_1,         (0x27),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dload_2,         (0x28),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dload_3,         (0x29),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(aload_0,         (0x2a),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(aload_1,         (0x2b),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(aload_2,         (0x2c),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(aload_3,         (0x2d),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(iaload,          (0x2e),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(laload,          (0x2f),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(faload,          (0x30),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(daload,          (0x31),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(aaload,          (0x32),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(baload,          (0x33),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(caload,          (0x34),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(saload,          (0x35),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(istore,          (0x36, 0),          (0xff, 0),                ("_%u", bytes[1]),                   (codes[0], args[0].u))
    paras_make_instr(lstore,          (0x37, 0),          (0xff, 0),                ("_%u", bytes[1]),                   (codes[0], args[0].u))
    paras_make_instr(fstore,          (0x38, 0),          (0xff, 0),                ("_%u", bytes[1]),                   (codes[0], args[0].u))
    paras_make_instr(dstore,          (0x39, 0),          (0xff, 0),                ("_%u", bytes[1]),                   (codes[0], args[0].u))
    paras_make_instr(astore,          (0x3a, 0),          (0xff, 0),                ("_%u", bytes[1]),                   (codes[0], args[0].u))
    paras_make_instr(istore_0,        (0x3b),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(istore_1,        (0x3c),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(istore_2,        (0x3d),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(istore_3,        (0x3e),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lstore_0,        (0x3f),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lstore_1,        (0x40),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lstore_2,        (0x41),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lstore_3,        (0x42),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fstore_0,        (0x43),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fstore_1,        (0x44),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fstore_2,        (0x45),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fstore_3,        (0x46),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dstore_0,        (0x47),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dstore_1,        (0x48),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dstore_2,        (0x49),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dstore_3,        (0x4a),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(astore_0,        (0x4b),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(astore_1,        (0x4c),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(astore_2,        (0x4d),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(astore_3,        (0x4e),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(iastore,         (0x4f),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lastore,         (0x50),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fastore,         (0x51),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dastore,         (0x52),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(aastore,         (0x53),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(bastore,         (0x54),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(castore,         (0x55),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(sastore,         (0x56),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(pop,             (0x57),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(pop2,            (0x58),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dup,             (0x59),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dup_x1,          (0x5a),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dup_x2,          (0x5b),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dup2,            (0x5c),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dup2_x1,         (0x5d),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dup2_x2,         (0x5e),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(swap,            (0x5f),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(iadd,            (0x60),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(ladd,            (0x61),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fadd,            (0x62),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dadd,            (0x63),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(isub,            (0x64),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lsub,            (0x65),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fsub,            (0x66),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dsub,            (0x67),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(imul,            (0x68),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lmul,            (0x69),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fmul,            (0x6a),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dmul,            (0x6b),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(idiv,            (0x6c),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(ldiv,            (0x6d),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fdiv,            (0x6e),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(ddiv,            (0x6f),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(irem,            (0x70),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lrem,            (0x71),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(frem,            (0x72),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(drem,            (0x73),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(ineg,            (0x74),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lneg,            (0x75),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fneg,            (0x76),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dneg,            (0x77),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(ishl,            (0x78),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lshl,            (0x79),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(ishr,            (0x7a),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lshr,            (0x7b),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(iushr,           (0x7c),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lushr,           (0x7d),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(iand,            (0x7e),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(land,            (0x7f),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(ior,             (0x80),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lor,             (0x81),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(ixor,            (0x82),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lxor,            (0x83),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(iinc,            (0x84, 0, 0),       (0xff, 0, 0),             ("_%u, %i", bytes[1], bytes[2]),     (codes[0], args[0].u, args[1].i))
    paras_make_instr(i2l,             (0x85),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(i2f,             (0x86),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(i2d,             (0x87),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(l2i,             (0x88),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(l2f,             (0x89),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(l2d,             (0x8a),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(f2i,             (0x8b),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(f2l,             (0x8c),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(f2d,             (0x8d),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(d2i,             (0x8e),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(d2l,             (0x8f),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(d2f,             (0x90),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(i2b,             (0x91),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(i2c,             (0x92),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(i2s,             (0x93),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lcmp,            (0x94),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fcmpl,           (0x95),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(fcmpg,           (0x96),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dcmpl,           (0x97),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dcmpg,           (0x98),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(ifeq,            (0x99, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(ifne,            (0x9a, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(iflt,            (0x9b, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(ifge,            (0x9c, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(ifgt,            (0x9d, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(ifle,            (0x9e, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(if_icmpeq,       (0x9f, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(if_icmpne,       (0xa0, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(if_icmplt,       (0xa1, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(if_icmpge,       (0xa2, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(if_icmpgt,       (0xa3, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(if_icmple,       (0xa4, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(if_acmpeq,       (0xa5, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(if_acmpne,       (0xa6, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(goto,            (0xa7, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(jsr,             (0xa8, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(ret,             (0xa9, 0),          (0xff, 0),                ("_%u", bytes[1]),                   (codes[0], args[0].u))
    paras_make_instr_x(tableswitch,   (0xaa),             (0xff),                   _jvm_tableswitch_f,                  _jvm_tableswitch_e)
    paras_make_instr_x(lookupswitch,  (0xab),             (0xff),                   _jvm_lookupswitch_f,                 _jvm_lookupswitch_e)
    paras_make_instr(ireturn,         (0xac),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(lreturn,         (0xad),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(freturn,         (0xae),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(dreturn,         (0xaf),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(areturn,         (0xb0),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(return,          (0xb1),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(getstatic,       (0xb2, 0, 0),       (0xff, 0, 0),             ("#%u", _op_u16_f(1)),               (codes[0], _op_u16_e(0)))
    paras_make_instr(putstatic,       (0xb3, 0, 0),       (0xff, 0, 0),             ("#%u", _op_u16_f(1)),               (codes[0], _op_u16_e(0)))
    paras_make_instr(getfield,        (0xb4, 0, 0),       (0xff, 0, 0),             ("#%u", _op_u16_f(1)),               (codes[0], _op_u16_e(0)))
    paras_make_instr(putfield,        (0xb5, 0, 0),       (0xff, 0, 0),             ("#%u", _op_u16_f(1)),               (codes[0], _op_u16_e(0)))
    paras_make_instr(invokevirtual,   (0xb6, 0, 0),       (0xff, 0, 0),             ("#%u", _op_u16_f(1)),               (codes[0], _op_u16_e(0)))
    paras_make_instr(invokespecial,   (0xb7, 0, 0),       (0xff, 0, 0),             ("#%u", _op_u16_f(1)),               (codes[0], _op_u16_e(0)))
    paras_make_instr(invokestatic,    (0xb8, 0, 0),       (0xff, 0, 0),             ("#%u", _op_u16_f(1)),               (codes[0], _op_u16_e(0)))
    paras_make_instr(invokeinterface, (0xb9, 0, 0, 0, 0), (0xff, 0, 0, 0, 0xff),    ("#%u, %u", _op_u16_f(1), bytes[3]), (codes[0], _op_u16_e(0), args[1].u))
    paras_make_instr(invokedynamic,   (0xba, 0, 0, 0, 0), (0xff, 0, 0, 0xff, 0xff), ("#%u", _op_u16_f(1)),               (codes[0], _op_u16_e(0)))
    paras_make_instr(new,             (0xbb, 0, 0),       (0xff, 0, 0),             ("#%u", _op_u16_f(1)),               (codes[0], _op_u16_e(0)))
    paras_make_instr(newarray,        (0xbc, 0),          (0xff, 0),                ("%s", _arr_ty_f(1)),                (codes[0], _arr_ty_e(0)))
    paras_make_instr(anewarray,       (0xbd, 0, 0),       (0xff, 0, 0),             ("#%u", _op_u16_f(1)),               (codes[0], _op_u16_e(0)))
    paras_make_instr(arraylength,     (0xbe),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(athrow,          (0xbf),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(checkcast,       (0xc0, 0, 0),       (0xff, 0, 0),             ("#%u", _op_u16_f(1)),               (codes[0], _op_u16_e(0)))
    paras_make_instr(instanceof,      (0xc1, 0, 0),       (0xff, 0, 0),             ("#%u", _op_u16_f(1)),               (codes[0], _op_u16_e(0)))
    paras_make_instr(monitorenter,    (0xc2),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(monitorexit,     (0xc3),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(wide_iload,      (0xc4, 0x15, 0, 0),       (0xff, 0xff, 0, 0),       ("_%u", _op_u16_f(2)),                   (codes[0], codes[1], _op_u16_e(0)))
    paras_make_instr(wide_lload,      (0xc4, 0x16, 0, 0),       (0xff, 0xff, 0, 0),       ("_%u", _op_u16_f(2)),                   (codes[0], codes[1], _op_u16_e(0)))
    paras_make_instr(wide_fload,      (0xc4, 0x17, 0, 0),       (0xff, 0xff, 0, 0),       ("_%u", _op_u16_f(2)),                   (codes[0], codes[1], _op_u16_e(0)))
    paras_make_instr(wide_dload,      (0xc4, 0x18, 0, 0),       (0xff, 0xff, 0, 0),       ("_%u", _op_u16_f(2)),                   (codes[0], codes[1], _op_u16_e(0)))
    paras_make_instr(wide_aload,      (0xc4, 0x19, 0, 0),       (0xff, 0xff, 0, 0),       ("_%u", _op_u16_f(2)),                   (codes[0], codes[1], _op_u16_e(0)))
    paras_make_instr(wide_istore,     (0xc4, 0x36, 0, 0),       (0xff, 0xff, 0, 0),       ("_%u", _op_u16_f(2)),                   (codes[0], codes[1], _op_u16_e(0)))
    paras_make_instr(wide_lstore,     (0xc4, 0x37, 0, 0),       (0xff, 0xff, 0, 0),       ("_%u", _op_u16_f(2)),                   (codes[0], codes[1], _op_u16_e(0)))
    paras_make_instr(wide_fstore,     (0xc4, 0x38, 0, 0),       (0xff, 0xff, 0, 0),       ("_%u", _op_u16_f(2)),                   (codes[0], codes[1], _op_u16_e(0)))
    paras_make_instr(wide_dstore,     (0xc4, 0x39, 0, 0),       (0xff, 0xff, 0, 0),       ("_%u", _op_u16_f(2)),                   (codes[0], codes[1], _op_u16_e(0)))
    paras_make_instr(wide_astore,     (0xc4, 0x3a, 0, 0),       (0xff, 0xff, 0, 0),       ("_%u", _op_u16_f(2)),                   (codes[0], codes[1], _op_u16_e(0)))
    paras_make_instr(wide_iinc,       (0xc4, 0x84, 0, 0, 0, 0), (0xff, 0xff, 0, 0, 0, 0), ("_%u, %i", _op_u16_f(2), _op_i16_f(4)), (codes[0], codes[1], _op_u16_e(0), _op_i16_e(1)))
    paras_make_instr(wide_ret,        (0xc4, 0xa9, 0, 0),       (0xff, 0xff, 0, 0),       ("_%u", _op_u16_f(2)),                   (codes[0], codes[1], _op_u16_e(0)))
    paras_make_instr(multianewarray,  (0xc5, 0, 0, 0),    (0xff, 0, 0, 0),          ("#%u, %u", _op_u16_f(1), bytes[2]), (codes[0], _op_u16_e(0), args[1].u))
    paras_make_instr(ifnull,          (0xc6, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(ifnonnull,       (0xc7, 0, 0),       (0xff, 0, 0),             ("%+i", _op_i16_f(1)),               (codes[0], _op_i16_e(0)))
    paras_make_instr(goto_w,          (0xc8, 0, 0, 0, 0), (0xff, 0, 0, 0, 0),       ("%+i", _op_i32_f(1)),               (codes[0], _op_i32_e(0)))
    paras_make_instr(jsr_w,           (0xc9, 0, 0, 0, 0), (0xff, 0, 0, 0, 0),       ("%+i", _op_i32_f(1)),               (codes[0], _op_i32_e(0)))
    paras_make_instr(breakpoint,      (0xca),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_cb,  (0xcb),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_cc,  (0xcc),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_cd,  (0xcd),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_ce,  (0xce),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_cf,  (0xcf),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_d0,  (0xd0),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_d1,  (0xd1),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_d2,  (0xd2),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_d3,  (0xd3),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_d4,  (0xd4),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_d5,  (0xd5),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_d6,  (0xd6),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_d7,  (0xd7),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_d8,  (0xd8),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_d9,  (0xd9),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_da,  (0xda),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_db,  (0xdb),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_dc,  (0xdc),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_dd,  (0xdd),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_de,  (0xde),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_df,  (0xdf),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_e0,  (0xe0),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_e1,  (0xe1),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_e2,  (0xe2),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_e3,  (0xe3),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_e4,  (0xe4),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_e5,  (0xe5),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_e6,  (0xe6),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_e7,  (0xe7),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_e8,  (0xe8),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_e9,  (0xe9),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_ea,  (0xea),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_eb,  (0xeb),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_ec,  (0xec),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_ed,  (0xed),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_ee,  (0xee),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_ef,  (0xef),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_f0,  (0xf0),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_f1,  (0xf1),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_f2,  (0xf2),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_f3,  (0xf3),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_f4,  (0xf4),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_f5,  (0xf5),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_f6,  (0xf6),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_f7,  (0xf7),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_f8,  (0xf8),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_f9,  (0xf9),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_fa,  (0xfa),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_fb,  (0xfb),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_fc,  (0xfc),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(_unassigned_fd,  (0xfd),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(impdep1,         (0xfe),             (0xff),                   (""),                                (codes[0]))
    paras_make_instr(impdep2,         (0xff),             (0xff),                   (""),                                (codes[0]))
)
// }}}

// bipa(jvm_class) {{{
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
        , (void,), (u8, 1, getField)         /* Fieldref */
        , (void,), (u8, 2, getStatic)        /* Fieldref */
        , (void,), (u8, 3, putField)         /* Fieldref */
        , (void,), (u8, 4, putStatic)        /* Fieldref */
        , (void,), (u8, 5, invokeVirtual)    /* Methodref */
        , (void,), (u8, 6, invokeStatic)     /* Methodref */
        , (void,), (u8, 7, invokeSpecial)    /* Methodref */
        , (void,), (u8, 8, newInvokeSpecial) /* Methodref */
        , (void,), (u8, 9, invokeInterface)  /* InterfaceMethodref */
        )

bipa_struct(jvm_class_Class,              1, (u16be,/*Utf8*/), name_index)
bipa_struct(jvm_class_Fieldref,           2, (u16be,/*Class*/), class_index, (u16be,/*NameAndType*/), name_and_type_index)
bipa_struct(jvm_class_Methodref,          2, (u16be,/*Class*/), class_index, (u16be,/*NameAndType*/), name_and_type_index)
bipa_struct(jvm_class_InterfaceMethodref, 2, (u16be,/*Class*/), class_index, (u16be,/*NameAndType*/), name_and_type_index)
bipa_struct(jvm_class_String,             1, (u16be,/*Utf8*/), string_index)
bipa_struct(jvm_class_Integer,            1, (u32be,), bytes)
bipa_struct(jvm_class_Float,              1, (u32be,), bytes)
bipa_struct(jvm_class_Long,               1, (u64be,), bytes)
bipa_struct(jvm_class_Double,             1, (u64be,), bytes)
bipa_struct(jvm_class_NameAndType,        2, (u16be,/*Utf8*/), name_index, (u16be,/*Utf8*/), descriptor_index)
bipa_struct(jvm_class_Utf8,               2, (u16be,), length, (lstr, self->length), bytes)
bipa_struct(jvm_class_MethodHandle,       2, (union, jvm_class_reference_kind), reference_kind, (u16be,), /* see jvm_class_reference_kind */ reference_index)
bipa_struct(jvm_class_MethodType,         1, (u16be,/*Utf8*/), descriptor_index)
bipa_struct(jvm_class_InvokeDynamic,      2, (u16be,), bootstrap_method_attr_index, (u16be,/*NameAndType*/), name_and_type_index)

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

bipa_array(jvm_class_interfaces, (u16be,/*Class*/))

bipa_struct(jvm_class_attribute_info, 3
        , (u16be,/*Utf8*/), attribute_name_index
        , (u32be,), attribute_length
        , (lstr, self->attribute_length), info
        )
bipa_array(jvm_class_attribute_infos, (struct, jvm_class_attribute_info))

bipa_struct(jvm_class_field_info, 5
        , (packed, jvm_class_access_flags), access_flags
        , (u16be,/*Utf8*/), name_index
        , (u16be,/*Utf8*/), descriptor_index
        , (u16be,), attributes_count
        , (array, jvm_class_attribute_infos, k < self->attributes_count), attributes
        )
bipa_array(jvm_class_field_infos, (struct, jvm_class_field_info))

bipa_struct(jvm_class_method_info, 5
        , (packed, jvm_class_access_flags), access_flags
        , (u16be,/*Utf8*/), name_index
        , (u16be,/*Utf8*/), descriptor_index
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
        , (u16be,/*Class*/), this_class
        , (u16be,/*Class*/), super_class
        , (u16be,), interfaces_count
        , (array, jvm_class_interfaces, k < self->interfaces_count), interfaces
        , (u16be,), fields_count
        , (array, jvm_class_field_infos, k < self->fields_count), fields
        , (u16be,), methods_count
        , (array, jvm_class_method_infos, k < self->methods_count), methods
        , (u16be,), attributes_count
        , (array, jvm_class_attribute_infos, k < self->attributes_count), attributes
        )

bipa_struct(jvm_attr_ConstantValue, 1
        , (u16be,), constantvalue_index
        )
bipa_struct(jvm_attr_Code_exception, 4
        , (u16be,), start_pc
        , (u16be,), end_pc
        , (u16be,), handler_pc
        , (u16be,), catch_type
        )
bipa_array(jvm_attr_Code_exceptions, (struct, jvm_attr_Code_exception))
bipa_struct(jvm_attr_Code, 8
        , (u16be,), max_stack
        , (u16be,), max_locals
        , (u32be,), code_length
        , (lstr, self->code_length), code
        , (u16be,), exception_table_length
        , (array, jvm_attr_Code_exceptions, k < self->exception_table_length), exception_table
        , (u16be,), attributes_count
        , (array, jvm_class_attribute_infos, k < self->attributes_count), attributes
        )
// }}}

// https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-4.html
adapt_bipa_type(jvm_class)
adapt_bipa_type(jvm_attr_ConstantValue)
adapt_bipa_type(jvm_attr_Code)

typedef struct jvm_class_member {
    jvm_class* cls;
    struct jvm_class_access_flags* access_flags;
    u16* name_index;
    u16* descriptor_index;
    u16* attributes_count;
    struct jvm_class_attribute_infos* attributes;
} jvm_class_member;

buf jvm_list_fields(jvm_class cref cls);
void jvm_reserve_fields(jvm_class ref cls, sz const count);
jvm_class_member jvm_get_field(jvm_class cref cls, buf const name);
jvm_class_member jvm_add_field(jvm_class ref cls, buf const name);

buf jvm_list_methods(jvm_class cref cls);
void jvm_reserve_methods(jvm_class ref cls, sz const count);
jvm_class_member jvm_get_method(jvm_class cref cls, buf const name);
jvm_class_member jvm_add_method(jvm_class ref cls, buf const name);

#define jvm_list_attributes(__cls)  jvm_member_list_attributes (&(jvm_class_member){.cls= (__cls), .attributes_count= &(__cls)->attributes_count, .attributes= &(__cls)->attributes})
#define jvm_reserve_attributes(__cls, __count)  jvm_member_reserve_attributes(&(jvm_class_member){.cls= (__cls), .attributes_count= &(__cls)->attributes_count, .attributes= &(__cls)->attributes}, (__count))
#define jvm_get_attribute(__cls, __name)  jvm_member_get_attribute (&(jvm_class_member){.cls= (__cls), .attributes_count= &(__cls)->attributes_count, .attributes= &(__cls)->attributes}, (__name))
#define jvm_add_attribute(__cls, __name, __info)  jvm_member_add_attribute (&(jvm_class_member){.cls= (__cls), .attributes_count= &(__cls)->attributes_count, .attributes= &(__cls)->attributes}, (__name), (__info))

buf jvm_member_list_attributes(jvm_class_member cref mbr);
void jvm_member_reserve_attributes(jvm_class_member ref mbr, sz const count);
struct jvm_class_attribute_info* jvm_member_get_attribute(jvm_class_member cref mbr, buf const name);
struct jvm_class_attribute_info* jvm_member_add_attribute(jvm_class_member cref mbr, buf const name, buf const info);

#ifdef BIDOOF_IMPLEMENTATION

// private paras stuff {{{
bool _jvm_tableswitch_f(buf cref src, sz ref at, buf ref res) {
    // TODO: overrun checks
    char _buf[256], *_h = _buf;
    u8 const* bytes = src->ptr;
    while (++*at & 3);
    i32 def = _op_i32_f(*at); *at+= 4;
    i32 lo = _op_i32_f(*at); *at+= 4;
    i32 hi = _op_i32_f(*at); *at+= 4;
    _h+= sprintf(_h, "%+i, [%i..%i]", def, lo, hi);
    for (int n = 0; n < hi-lo+1; n++) {
        i32 off = _op_i32_f(*at); *at+= 4;
        _h+= sprintf(_h, ", %+i", off);
    }
    bufcat(res, (buf){.ptr= (u8*)_buf, .len= _h-_buf});
    return true;
}
bool _jvm_tableswitch_e(buf cref line, buf ref res) {
    // TODO: true scan failures (`at` not changed)
    sz at = 0;
    union paras_generic def_lo_hi[3];
    _paras_scan(line, &at, 0, "%+i, [%i..%i]", def_lo_hi);
    i32 const lo = def_lo_hi[1].i, hi = def_lo_hi[2].i;
    sz const isz = 1 + 3 + (3 + hi-lo+1) *4;
    if (res->cap <= res->len+isz && !dyarr_resize(res, res->len+isz)) exitf("OOM");
    res->ptr[res->len++] = 0xaa;
    if (res->len & 3) res->len+= 4-(res->len&3);
    poke32be(res, res->len, def_lo_hi[0].i); res->len+= 4;
    poke32be(res, res->len, lo); res->len+= 4;
    poke32be(res, res->len, hi); res->len+= 4;
    for (int n = 0; n < hi-lo+1; n++) {
        union paras_generic off;
        _paras_scan(line, &at, 0, ", %+i", &off);
        poke32be(res, res->len, off.i); res->len+= 4;
    }
    return true;
}
bool _jvm_lookupswitch_f(buf cref src, sz ref at, buf ref res) {
    // TODO: overrun checks
    char _buf[256], *_h = _buf;
    u8 const* bytes = src->ptr;
    while (++*at & 3);
    i32 def = _op_i32_f(*at); *at+= 4;
    i32 np = _op_i32_f(*at); *at+= 4;
    _h+= sprintf(_h, "%+i, [%u]", def, np);
    for (int n = 0; n < np; n++) {
        i32 mat = _op_i32_f(*at); *at+= 4;
        i32 off = _op_i32_f(*at); *at+= 4;
        _h+= sprintf(_h, ", (%i, %+i)", mat, off);
    }
    bufcat(res, (buf){.ptr= (u8*)_buf, .len= _h-_buf});
    return true;
}
bool _jvm_lookupswitch_e(buf cref line, buf ref res) {
    // TODO: true scan failures (`at` not changed)
    sz at = 0;
    union paras_generic def_np[3];
    _paras_scan(line, &at, 0, "%+i, [%u]", def_np);
    i32 const np = def_np[1].u;
    sz const isz = 1 + 3 + (2 + np*2) *4;
    if (res->cap <= res->len+isz && !dyarr_resize(res, res->len+isz)) exitf("OOM");
    res->ptr[res->len++] = 0xab;
    if (res->len & 3) res->len+= 4-(res->len&3);
    poke32be(res, res->len, def_np[0].i); res->len+= 4;
    poke32be(res, res->len, np); res->len+= 4;
    for (int n = 0; n < np; n++) {
        union paras_generic mat_off[2];
        _paras_scan(line, &at, 0, ", (%i, %+i)", mat_off);
        poke32be(res, res->len, mat_off[0].i); res->len+= 4;
        poke32be(res, res->len, mat_off[1].i); res->len+= 4;
    }
    return true;
}
// }}}

// fields {{{
buf jvm_list_fields(jvm_class cref cls) {
    buf r = {0};
    for (sz k = 0; k < cls->fields.len; k++) {
        struct jvm_class_field_info ref it = cls->fields.ptr+k;
        unsigned n = it->name_index-1;
        if (n < cls->constant_pool_count) {
            struct jvm_class_cp_info ref fo = cls->constant_pool.ptr+n;
            if (jvm_class_cp_info_tag_Utf8 == fo->tag) {
                bufcat(&r, (buf){.ptr= fo->val.Utf8.bytes, .len= fo->val.Utf8.length+1});
                r.ptr[r.len-1] = '\n';
            }
        }
    }
    return r;
}

void jvm_reserve_fields(jvm_class ref cls, sz const count) {
    if (cls->fields.cap < count && !dyarr_resize(&cls->fields, count)) exitf("OOM");
}

jvm_class_member jvm_get_field(jvm_class cref cls, buf const name) {
    for (sz k = 0; k < cls->fields.len; k++) {
        struct jvm_class_field_info ref it = cls->fields.ptr+k;
        unsigned n = it->name_index-1;
        if (n < cls->constant_pool_count) {
            struct jvm_class_cp_info ref fo = cls->constant_pool.ptr+n;
            if (jvm_class_cp_info_tag_Utf8 == fo->tag && !memcmp(name.ptr, fo->val.Utf8.bytes, fo->val.Utf8.length))
                return (jvm_class_member){
                    .cls= (void*)cls,
                    .access_flags= &it->access_flags,
                    .name_index= &it->name_index,
                    .descriptor_index= &it->descriptor_index,
                    .attributes_count= &it->attributes_count,
                    .attributes= &it->attributes,
                };
        }
    }
    exitf("field not found in class: '%.*s'", (unsigned)name.len, name.ptr);
}

jvm_class_member jvm_add_field(jvm_class ref cls, buf const name) {
    struct jvm_class_field_info ref it = dyarr_push(&cls->fields);
    if (!it) exitf("OOM");
    // TODO: add constant (name)
    cls->fields_count++;
    return (jvm_class_member){
        .cls= (void*)cls,
        .access_flags= &it->access_flags,
        .name_index= &it->name_index,
        .descriptor_index= &it->descriptor_index,
        .attributes_count= &it->attributes_count,
        .attributes= &it->attributes,
    };
}
// }}}

// methods {{{
buf jvm_list_methods(jvm_class cref cls) {
    buf r = {0};
    for (sz k = 0; k < cls->methods.len; k++) {
        struct jvm_class_method_info ref it = cls->methods.ptr+k;
        unsigned n = it->name_index-1;
        if (n < cls->constant_pool_count) {
            struct jvm_class_cp_info ref fo = cls->constant_pool.ptr+n;
            if (jvm_class_cp_info_tag_Utf8 == fo->tag) {
                bufcat(&r, (buf){.ptr= fo->val.Utf8.bytes, .len= fo->val.Utf8.length+1});
                r.ptr[r.len-1] = '\n';
            }
        }
    }
    return r;
}

void jvm_reserve_methods(jvm_class ref cls, sz const count) {
    if (cls->methods.cap < count && !dyarr_resize(&cls->methods, count)) exitf("OOM");
}

jvm_class_member jvm_get_method(jvm_class cref cls, buf const name) {
    for (sz k = 0; k < cls->methods.len; k++) {
        struct jvm_class_method_info ref it = cls->methods.ptr+k;
        unsigned n = it->name_index-1;
        if (n < cls->constant_pool_count) {
            struct jvm_class_cp_info ref fo = cls->constant_pool.ptr+n;
            if (jvm_class_cp_info_tag_Utf8 == fo->tag && !memcmp(name.ptr, fo->val.Utf8.bytes, fo->val.Utf8.length))
                return (jvm_class_member){
                    .cls= (void*)cls,
                    .access_flags= &it->access_flags,
                    .name_index= &it->name_index,
                    .descriptor_index= &it->descriptor_index,
                    .attributes_count= &it->attributes_count,
                    .attributes= &it->attributes,
                };
        }
    }
    exitf("method not found in class: '%.*s'", (unsigned)name.len, name.ptr);
}

jvm_class_member jvm_add_method(jvm_class ref cls, buf const name) {
    struct jvm_class_method_info ref it = dyarr_push(&cls->methods);
    if (!it) exitf("OOM");
    // TODO: add constant (name)
    cls->methods_count++;
    return (jvm_class_member){
        .cls= (void*)cls,
        .access_flags= &it->access_flags,
        .name_index= &it->name_index,
        .descriptor_index= &it->descriptor_index,
        .attributes_count= &it->attributes_count,
        .attributes= &it->attributes,
    };
}
// }}}

// attributes {{{
buf jvm_member_list_attributes(jvm_class_member cref mbr) {
    buf r = {0};
    for (sz k = 0; k < mbr->attributes->len; k++) {
        struct jvm_class_attribute_info ref it = mbr->attributes->ptr+k;
        unsigned n = it->attribute_name_index-1;
        if (n < mbr->cls->constant_pool_count) {
            struct jvm_class_cp_info ref fo = mbr->cls->constant_pool.ptr+n;
            if (jvm_class_cp_info_tag_Utf8 == fo->tag) {
                bufcat(&r, (buf){.ptr= fo->val.Utf8.bytes, .len= fo->val.Utf8.length+1});
                r.ptr[r.len-1] = '\n';
            }
        }
    }
    return r;
}

void jvm_member_reserve_attributes(jvm_class_member ref mbr, sz const count) {
    if (mbr->attributes->cap < count && !dyarr_resize(mbr->attributes, count)) exitf("OOM");
}

struct jvm_class_attribute_info* jvm_member_get_attribute(jvm_class_member cref mbr, buf const name) {
    for (sz k = 0; k < mbr->attributes->len; k++) {
        struct jvm_class_attribute_info ref it = mbr->attributes->ptr+k;
        unsigned n = it->attribute_name_index-1;
        if (n < mbr->cls->constant_pool_count) {
            struct jvm_class_cp_info ref fo = mbr->cls->constant_pool.ptr+n;
            if (jvm_class_cp_info_tag_Utf8 == fo->tag && !memcmp(name.ptr, fo->val.Utf8.bytes, fo->val.Utf8.length))
                return it;
        }
    }
    exitf("attribute not found: '%.*s'", (unsigned)name.len, name.ptr);
}

struct jvm_class_attribute_info* jvm_member_add_attribute(jvm_class_member cref mbr, buf const name, buf const info) {
    struct jvm_class_attribute_info* r = dyarr_push(mbr->attributes);
    // TODO: (better) add constant
    struct jvm_class_cp_info* na = dyarr_push(&mbr->cls->constant_pool);
    if (!r || !na) exitf("OOM");
    na->tag = jvm_class_cp_info_tag_Utf8;
    na->val.Utf8.length = name.len;
    na->val.Utf8.bytes = bufcpy(name).ptr;
    r->attribute_name_index = ++mbr->cls->constant_pool_count;
    r->attribute_length = info.len;
    r->info = bufcpy(info).ptr;
    ++*mbr->attributes_count;
    return r;
}
// }}}

#endif // BIDOOF_IMPLEMENTATION

#endif // __BIDOOF_T_JVM__
