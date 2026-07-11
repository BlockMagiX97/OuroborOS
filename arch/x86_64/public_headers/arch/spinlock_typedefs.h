#ifndef OUROBOROS_ARCH_X86_64_SPINLOCK_TYPEDEFS_H
#define OUROBOROS_ARCH_X86_64_SPINLOCK_TYPEDEFS_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
		struct {
			uint32_t locked; // (only lsb is used for lock rest is reserved for metadata)
		};
} spinlock_t;


typedef struct __attribute__((packed)) {
		struct {
			uint32_t num_readers;
			uint8_t want_write : 1;
		};
} rwspinlock_t;

#define SPINLOCK_INIT_LOCKED \
	((spinlock_t){ \
		.locked=1 \
	})

#define SPINLOCK_INIT_UNLOCKED \
	((spinlock_t){ \
		.locked=0 \
	})

#define RWSPINLOCK_INIT_WRITE_LOCKED \
	((rwspinlock_t){ \
		.num_readers=0, \
	 	.want_write=1 \
	 })

#define RWSPINLOCK_INIT_READ_LOCKED \
	((rwspinlock_t){ \
		.num_readers=1, \
	 	.want_write=0 \
	 })

#define RWSPINLOCK_INIT_UNLOCKED \
	((rwspinlock_t){ \
		.num_readers=0, \
	 	.want_write=0 \
	 })

#endif
