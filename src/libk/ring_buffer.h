#ifndef OUROBOROS_LIBK_RINGBUFFER_H
#define OUROBOROS_LIBK_RINGBUFFER_H
#include <stdint.h>

#define MAKE_RINGBUFFER_DECL(type) \
	struct ringbuffer_##type { \
		type* buf; \
		uint32_t cap; \
	};

#define RINGBUFFER(type) \
	struct ringbuffer_##type

// needs pointer to ringbuffer and index
#define RING_BUFFER_INDEX(ringbuffer, index) \
	((ringbuffer)->buf[(index) % (ringbuffer)->cap])

#endif
