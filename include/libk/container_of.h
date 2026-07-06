#ifndef OUROBOROS_LIBK_CONTAINER_OF
#define OUROBOROS_LIBK_CONTAINER_OF
#include <stddef.h>
#include <compiler/wrap_builtin.h>

#define __container_of(ptr, type, member) ({\
STATIC_ASSERT(SAME_TYPE(*(ptr), ((type *)0)->member) ||	SAME_TYPE(*(ptr), void), "pointer type mismatch in container_of()"); \
((type *)((void *)(ptr) - offsetof(type, member))); })

#define container_of(ptr, type, member)				\
_Generic(ptr,							\
	const typeof(*(ptr)) *: ((const type *)__container_of(ptr, type, member)),\
	default: ((type *)__container_of(ptr, type, member))	\
)
#endif
