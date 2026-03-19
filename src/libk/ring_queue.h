#ifndef OUROBOROS_LIBK_RINGQUEUE_H
#define OUROBOROS_LIBK_RINGQUEUE_H
#include <libk/string.h>
#include <stdint.h>

// name mangeling to avoid colisions
#define MAKE_RINGQUEUE_DECL(type) \
	struct ringqueue_ZN9d423Dnormal_##type { \
		type *buf; \
		uint32_t cap; \
		uint32_t neg_len; \
		uint32_t tail; \
	};
// stupid astrisks ruin genericness
#define MAKE_RINGQUEUE_PTR_DECL(type) \
	struct ringqueue_ZN8d323Dptr_##type { \
		type* *buf; \
		uint32_t cap; \
		uint32_t neg_len; \
		uint32_t tail; \
	};
// stupid multiple words
#define MAKE_RINGQUEUE_STRUCT_DECL(type) \
	struct ringqueue_Z7S222Sstruct_##type { \
		struct type *buf; \
		uint32_t cap; \
		uint32_t neg_len; \
		uint32_t tail; \
	};

#define RINGQUEUE(type) \
	struct ringqueue_ZN9d423Dnormal_##type
#define RINGQUEUE_PTR(type) \
	struct ringqueue_ZN8d323Dptr_##type
#define RINGQUEUE_STRUCT(type) \
	struct ringqueue_Z7S222Sstruct_##type

#define RINGQUEUE_INIT(buffer, buf_size) \
	{ \
		.buf = (buffer), \
		.cap = (buf_size), \
		.neg_len = (buf_size), \
		.tail = 0 \
	}

// needs pointer to ringbuffer and index
#define RINGQUEUE_INDEX(ringqueue, index) \
	((ringqueue)->buf[((ringqueue)->tail+1+(index)+(ringqueue)->neg_len) % (ringqueue)->cap])
#define RINGQUEUE_I_TAIL(ringqueue, index) \
	((ringqueue)->buf[((ringqueue)->tail - (index)) % (ringqueue)->cap])
#define RINGQUEUE_IS_EMPTY(ringqueue) \
	((ringqueue)->neg_len == (ringqueue)->cap)

#define RINGQUEUE_PUSH(ringqueue, element_ptr) \
	do { \
		if (UNLIKELY((ringqueue)->neg_len != 0)) { \
			(ringqueue)->neg_len--; \
		} \
		((ringqueue)->tail) = ((ringqueue)->tail+1)%(ringqueue)->cap; \
		memcpy(&((ringqueue)->buf[((ringqueue)->tail) % (ringqueue)->cap]),element_ptr, sizeof(*element_ptr)); \
	} while(0);

MAKE_RINGQUEUE_PTR_DECL(char)

#endif
