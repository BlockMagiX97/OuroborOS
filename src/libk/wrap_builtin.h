#ifndef OUROBOROS_LIBK_WRAP_BUILTIN_H
#define OUROBOROS_LIBK_WRAP_BUILTIN_H

#define STATIC_ASSERT(expr, error) _Static_assert(expr, error)
#define LIKELY(x)	__builtin_expect(!!(x), 1)
#define UNLIKELY(x) 	__builtin_expect(!!(x), 0)
#define UNREACHABLE	__builtin_unreachable()
#define MUL_OVERFLOW(x, y, res)	(__builtin_mul_overflow((x), (y), (res)))
#define same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#endif
