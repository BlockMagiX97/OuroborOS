#ifndef OUROBOROS_ARCH_SPINLOCK_H
#define OUROBOROS_ARCH_SPINLOCK_H
#include <stdbool.h>
#include <stdint.h>

#include <arch/typedefs.h>

#define CACHE_LINE_SIZE 64

typedef struct __attribute__((packed)) {
		struct {
			uint32_t locked; // (only lsb is used for lock rest is reserved for metadata)
		};
} spinlock_t;

// to add reader check if want_write is 0 increment num_readers else spin until it is 0 and then increment
// num_readers
// to release reader decrement num_readers
//
// to add a write check if want_write is 1 if yes spin until 0 then set it your self, then wait until num_readers is 0 then you have aquired your lock
// to release writer set want_write
// to 0

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
	 	.locked=1 \
	 })


bool spinlock_test_acquired(spinlock_t *sl);
void spinlock_acquire(spinlock_t *sl, flags_t *flags);
bool spinlock_try_acquire(spinlock_t *sl, flags_t *flags);
void spinlock_release(spinlock_t *sl, flags_t *flags);

void rwspinlock_acquire_read(rwspinlock_t *sl, flags_t *flags);
void rwspinlock_release_read(rwspinlock_t *sl, flags_t *flags);
void rwspinlock_acquire_write(rwspinlock_t *sl, flags_t *flags);
void rwspinlock_release_write(rwspinlock_t *sl, flags_t *flags);

#endif
