#ifndef OUROBOROS_LIBK_WRAP_BUILTIN_H
#define OUROBOROS_LIBK_WRAP_BUILTIN_H

#define LIKELY(x)	__builtin_expect(!!(x), 1)
#define UNLIKELY(x) 	__builtin_expect(!!(x), 0)
#define UNREACHABLE	__builtin_unreachable()
#define MUL_OVERFLOW(x, y, res)	(__builtin_mul_overflow((x), (y), res))

#endif
