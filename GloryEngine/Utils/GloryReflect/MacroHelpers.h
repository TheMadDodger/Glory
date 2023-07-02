#pragma once

#define STRINGIZE(arg) STRINGIZE1(arg)
#define STRINGIZE1(arg) STRINGIZE2(arg)
#define STRINGIZE2(arg) STRINGIZE3(arg)
#define STRINGIZE3(arg) #arg

#define CAT_TOKENS(x, y) CAT_PASS1((x, y))
#define CAT_PASS1(x) PRIMITIVE_CAT x
#define PRIMITIVE_CAT(x, y) x ## y

#define FOREACH_0(m)
#define FOREACH_1(m, x1) m(x1)
#define FOREACH_2(m, x1, x2) m(x1) m(x2)
#define FOREACH_3(m, x1, x2, x3) m(x1) m(x2) m(x3)
#define FOREACH_4(m, x1, x2, x3, x4) m(x1) m(x2) m(x3) m(x4)
#define FOREACH_5(m, x1, x2, x3, x4, x5) m(x1) m(x2) m(x3) m(x4) m(x5)
#define FOREACH_6(m, x1, x2, x3, x4, x5, x6) m(x1) m(x2) m(x3) m(x4) m(x5) m(x6)
#define FOREACH_7(m, x1, x2, x3, x4, x5, x6, x7) m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7)
#define FOREACH_8(m, x1, x2, x3, x4, x5, x6, x7, x8) m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8)
#define FOREACH_9(m, x1, x2, x3, x4, x5, x6, x7, x8, x9) m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8) m(x9)
#define FOREACH_10(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10) m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8) m(x9) m(x10)
#define FOREACH_11(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11) m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8) m(x9) m(x10) m(x11)
#define FOREACH_12(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12) m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8) m(x9) m(x10) m(x11) m(x12)
#define FOREACH_13(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13) m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8) m(x9) m(x10) m(x11) m(x12) m(x13)
#define FOREACH_14(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14) m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8) m(x9) m(x10) m(x11) m(x12) m(x13) m(x14)
#define FOREACH_15(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15) m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8) m(x9) m(x10) m(x11) m(x12) m(x13) m(x14) m(x15)
#define FOREACH_16(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16) m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8) m(x9) m(x10) m(x11) m(x12) m(x13) m(x14) m(x15) m(x16)
#define FOREACH_17(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17) m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8) m(x9) m(x10) m(x11) m(x12) m(x13) m(x14) m(x15) m(x16) m(x17)
#define FOREACH_18(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18) m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8) m(x9) m(x10) m(x11) m(x12) m(x13) m(x14) m(x15) m(x16) m(x17) m(x18)
#define FOREACH_19(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19) m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8) m(x9) m(x10) m(x11) m(x12) m(x13) m(x14) m(x15) m(x16) m(x17) m(x18) m(x19)
#define FOREACH_20(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20) m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8) m(x9) m(x10) m(x11) m(x12) m(x13) m(x14) m(x15) m(x16) m(x17) m(x18) m(x19) m(x20)

#define NARGS(...) NARGS_PASS2(NARGS_PASS1(__VA_ARGS__))
#define NARGS_PASS1(...) unused, __VA_ARGS__
#define NARGS_PASS2(...) NARGS_PASS4(NARGS_PASS3(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define NARGS_PASS3(_unused,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,VAL, ...) VAL
#define NARGS_PASS4(x) x

#define FOR_EACH_PASS1(m, x) m x
#define FOR_EACH(macro, ...) FOR_EACH_PASS1(CAT_TOKENS(FOREACH_, NARGS(__VA_ARGS__)), (macro, __VA_ARGS__))


#define ARGTYPE(x) ARGTYPE_PASS2(ARGTYPE_PASS1 x,)
#define ARGTYPE_PASS1(...) (__VA_ARGS__),
#define ARGTYPE_PASS2(...) ARGTYPE_PASS3((__VA_ARGS__))
#define ARGTYPE_PASS3(x)   ARGTYPE_PASS4 x
#define ARGTYPE_PASS4(x, ...) REM x
#define REM(...) __VA_ARGS__

#define ARGPAIR(x) ARGTYPE(x) ARGNAME(x)

#define ARGNAME(x) ARGNAME_PASS1(EAT x)
#define ARGNAME_PASS1(x) VAL x
#define EAT(...)

#define ARGNAME_AS_STRING(x) STRINGIZE(ARGNAME(x))
#define ARGTYPE_AS_STRING(x) STRINGIZE(ARGTYPE(x))

#define VAL(x) x


#define SPLIT_ARG1(x) VAL x
#define SPLIT_ARG1_1(x) VAL x

#define SPLIT_ARG2(x) SPLIT_ARG2_1(EAT x)
#define SPLIT_ARG2_1(x) VAL x
