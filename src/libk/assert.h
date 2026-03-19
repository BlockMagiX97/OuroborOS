#include <libk/util.h>
#include <libk/wrap_builtin.h>
#include <screen/text_screen_mgr.h>

// can be in an include gaurd
#ifndef OUROBOROS_ASSERT_H
#define OUROBOROS_ASSERT_H

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
			weak_puts("\033[0m\033[38;2;255;0;0mSafety assertion failed:" __FILE__ ":" STRINGIFY_VALUE(__LINE__)": " #expr "\033[0m"); \
			hcf(); \
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
			weak_puts("\033[38;2;255;0;0mAssertion failed:" __FILE__ ":" STRINGIFY_VALUE(__LINE__) ": " #expr "\033[0m"); \
			hcf(); \
		} \
	} while (0)
#endif
