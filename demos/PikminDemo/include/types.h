#ifndef _TYPES_H
#define _TYPES_H

#include <dolphin/types.h>

// r2  is 803F0200
// r13 is 803E4D20

#ifndef uint
typedef unsigned int uint;
#endif

typedef long double f128;
typedef volatile f128 vf128;

#if !defined(__cplusplus) && !defined(_WCHAR_T_DEFINED)
typedef u16 wchar_t;
#endif

// Workaround to introduce scoped enums (A feature of C++11 and onward).
#define BEGIN_ENUM_TYPE(name) \
	struct name {             \
		typedef

#define END_ENUM_TYPE \
	Type;             \
	}

// Random and useful macros
#ifndef PATH_MAX
#define PATH_MAX (256)
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

// Flag manipulation macros
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(o) (sizeof((o)) / sizeof(*(o)))
#endif

#define ALIGN_PREV(X, N)     ((X) & ~((N) - 1))
#define ALIGN_NEXT(X, N)     ALIGN_PREV(((X) + (N) - 1), N)
#define IS_NOT_ALIGNED(X, N) (((X) & ((N) - 1)) != 0)

#ifndef ATTRIBUTE_ALIGN
#define ATTRIBUTE_ALIGN(num) __attribute__((aligned(num)))
#endif

#ifdef __MWERKS__
#define BUMP_REGISTER(reg) \
	{                      \
		asm { mr reg, reg }   \
	}
#else
#define BUMP_REGISTER(reg) (void)0
#endif

// clang-format off

#define FORCE_DONT_INLINE \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0;

#define REPEAT1(x)  x
#define REPEAT2(x)  REPEAT1(x); x
#define REPEAT3(x)  REPEAT2(x); x
#define REPEAT4(x)  REPEAT3(x); x
#define REPEAT5(x)  REPEAT4(x); x
#define REPEAT6(x)  REPEAT5(x); x
#define REPEAT7(x)  REPEAT6(x); x
#define REPEAT8(x)  REPEAT7(x); x
#define REPEAT9(x)  REPEAT8(x); x
#define REPEAT10(x) REPEAT9(x); x
#define REPEAT11(x) REPEAT10(x); x
#define REPEAT12(x) REPEAT11(x); x

#define REPEAT(x, n) REPEAT##n(x)

// Add an unused local variable to pad the stack by some number of words
#define STACK_PAD_VAR(n) do { int pad[n]; } while (0)

// Create a temporary struct to pad the stack by some number of words
#define STACK_PAD_STRUCT(n) do { (void)sizeof(int[(n)]); } while (0)

// Add an unused variable in an inline function to pad the stack by some number of words
inline void padStack(void) { int pad = 0; }
#define STACK_PAD_INLINE(n) REPEAT(padStack(), n)

// Uses a ternary to pad the stack by some number of words
#define STACK_PAD_TERNARY(expr, n) do { (void)(expr); (void)sizeof(int[(n)]); } while (0)

// clang-format on

#ifdef __MWERKS__
#define WEAKFUNC        __declspec(weak)
#define DECL_SECT(name) __declspec(section name)
#define ASM             asm
#else
#define WEAKFUNC
#define DECL_SECT(name)
#define ASM
#endif

#define INIT  DECL_SECT(".init")
#define CTORS DECL_SECT(".ctors")

// Disable clangd warnings
#ifdef __clang__
// Allow string literals to be converted to char*
#pragma clang diagnostic ignored "-Wc++11-compat-deprecated-writable-strings"
#endif

#endif // _TYPES_H
