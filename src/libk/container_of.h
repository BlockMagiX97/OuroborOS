#include <stddef.h>
#include <libk/wrap_builtin.h>

#define typeof_member(T, m)	typeof(((T*)0)->m)


#define __container_of(ptr, type, member) ({\
STATIC_ASSERT(same_type(*(ptr), ((type *)0)->member) ||	same_type(*(ptr), void), "pointer type mismatch in container_of()"); \
((type *)((void *)(ptr) - offsetof(type, member))); })

#define container_of(ptr, type, member)				\
_Generic(ptr,							\
	const typeof(*(ptr)) *: ((const type *)__container_of(ptr, type, member)),\
	default: ((type *)__container_of(ptr, type, member))	\
)


