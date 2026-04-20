#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <arch/x86_64/kpanic.h>

#include <arch/x86_64/spinlock.h>

extern void acquireLock(spinlock_t *sl);
extern void acquireLockWithFlags(spinlock_t *sl, uint64_t *flags);
extern bool acquireTryLock(spinlock_t *sl);
extern bool acquireTryLockWithFlags(spinlock_t *sl, uint64_t *flags);
extern void releaseLock(spinlock_t *sl);
extern void releaseLockWithFlags(spinlock_t *sl, uint64_t *flags);
extern bool testLock(spinlock_t *sl);

bool spinlock_test_acquired(spinlock_t *sl) {
	return testLock(sl);
}
void spinlock_acquire(spinlock_t *sl, uint64_t *flags) {
	if (flags == NULL)
		acquireLock(sl);
	else
		acquireLockWithFlags(sl, flags);
}
bool spinlock_try_acquire(spinlock_t *sl, uint64_t *flags) {
	if (flags == NULL)
		return acquireTryLock(sl);
	else
		return acquireTryLockWithFlags(sl, flags);
}
void spinlock_release(spinlock_t *sl, uint64_t *flags) {
	if (!spinlock_test_acquired(sl)) {
		kpanic("Attempted to release spinlock that was already released");
	}
	if (flags == NULL)
		releaseLock(sl);
	else
		releaseLockWithFlags(sl, flags);
}

extern void acquireReadLock(rwspinlock_t *sl);
extern void acquireReadLockWithFlags(rwspinlock_t *sl, uint64_t *flags);
extern void releaseReadLock(rwspinlock_t *sl);
extern void releaseReadLockWithFlags(rwspinlock_t *sl, uint64_t *flags);

extern void acquireWriteLock(rwspinlock_t *sl);
extern void acquireWriteLockWithFlags(rwspinlock_t *sl, uint64_t *flags);
extern void releaseWriteLock(rwspinlock_t *sl);
extern void releaseWriteLockWithFlags(rwspinlock_t *sl, uint64_t *flags);

// wrappers for easier implementation of debug features
void rwspinlock_acquire_read(rwspinlock_t *sl, uint64_t *flags) {
	if (flags == NULL)
		acquireReadLock(sl);
	else
		acquireReadLockWithFlags(sl, flags);
}
void rwspinlock_release_read(rwspinlock_t *sl, uint64_t *flags) {
	if (flags == NULL)
		releaseReadLock(sl);
	else
		releaseReadLockWithFlags(sl, flags);
}

void rwspinlock_acquire_write(rwspinlock_t *sl, uint64_t *flags) {
	if (flags == NULL)
		acquireWriteLock(sl);
	else
		acquireWriteLockWithFlags(sl, flags);
}
void rwspinlock_release_write(rwspinlock_t *sl, uint64_t *flags) {
	if ((sl->want_write & 1) != 1) {
		kpanic("Attempted to release write on rwspinlock that was already released");
	}
	if (flags == NULL)
		releaseWriteLock(sl);
	else
		releaseWriteLockWithFlags(sl, flags);
}
