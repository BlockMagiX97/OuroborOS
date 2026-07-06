#include <arch/curr/hcf.h>
#include <arch/curr/kpanic.h>
#include <compiler/wrap_builtin.h>
#include <output/debug.h>

// can be in an include gaurd
#ifndef OUROBOROS_LIBK_ASSERT_H
#define OUROBOROS_LIBK_ASSERT_H

#define ___STRINGIFY_VALUE___(x) #x
#define STRINGIFY_VALUE(x) ___STRINGIFY_VALUE___(x)

// assert that is always active
#define safety_assert_no_print(expr) \
	do { \
		if (UNLIKELY(!(expr))) { \
			hcf(); \
		} \
	} while (0)

#define safety_assert(expr) \
	do { \
		if (UNLIKELY(!(expr))) { \
			kpanic("Safety assertion failed:" __FILE__ ":"  STRINGIFY_VALUE(__LINE__)": " #expr , ""); \
		} \
	} while (0)
#endif

// so it can switch (standart states that if NDEBUG is defined at last include point it does nothing otherwise assert)
#ifdef assert_no_print
#undef assert_no_print
#endif

#ifdef NDEBUG
// make it defined but do nothing
#define assert_no_print(expr)
#else
// define assert
#define assert_no_print(expr) \
	do { \
		if (UNLIKELY(!(expr))) { \
			hcf(); \
		} \
	} while (0)
#endif

#ifdef assert
#undef assert
#endif

#ifdef NDEBUG
// make it defined but do nothing
#define assert(expr)
#else
// define assert
#define assert(expr) \
	do { \
		if (UNLIKELY(!(expr))) { \
			kpanic("Assertion failed:" __FILE__ ":"  STRINGIFY_VALUE(__LINE__)": " #expr , ""); \
		} \
	} while (0)
#endif
