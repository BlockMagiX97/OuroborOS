#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <libk/assert.h>
#include <libk/stdio.h>
#include <libk/string.h>
#include <libk/util.h>

#include <screen/font.h>
#include <screen/screen.h>
#include <screen/text_screen_mgr.h>

#include <arch/curr/init.h>
#include <arch/curr/spinlock.h>

// clang-format off
//
// Set the base revision to 4, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.
__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(4);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.
__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// clang-format on
void kmain(void) {
	// Ensure the bootloader actually understands our base revision (see spec).
	if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
		hcf();
	}

	if (init_screen() < 0) {
		hcf();
	}
	init_boot_cpu();

	uint64_t flags[5];
	rwspinlock_t rwlock;
	rwspinlock_acquire_read(&rwlock, flags+0);
	printf_limited("num_readers: %d", rwlock.num_readers);
	printf_limited("want_write: %hhd", rwlock.want_write);
	printf_limited("");
	rwspinlock_acquire_read(&rwlock, flags+1);
	printf_limited("num_readers: %d", rwlock.num_readers);
	printf_limited("want_write: %hhd", rwlock.want_write);
	printf_limited("");
	rwspinlock_acquire_read(&rwlock, flags+2);
	printf_limited("num_readers: %d", rwlock.num_readers);
	printf_limited("want_write: %hhd", rwlock.want_write);
	printf_limited("");
	rwspinlock_release_read(&rwlock, flags+1);
	printf_limited("num_readers: %d", rwlock.num_readers);
	printf_limited("want_write: %hhd", rwlock.want_write);
	printf_limited("");
	rwspinlock_release_read(&rwlock, flags+2);
	printf_limited("num_readers: %d", rwlock.num_readers);
	printf_limited("want_write: %hhd", rwlock.want_write);
	printf_limited("");
	rwspinlock_release_read(&rwlock, flags+0);
	printf_limited("num_readers: %d", rwlock.num_readers);
	printf_limited("want_write: %hhd", rwlock.want_write);
	printf_limited("");
	rwspinlock_acquire_write(&rwlock, flags+3);
	printf_limited("num_readers: %d", rwlock.num_readers);
	printf_limited("want_write: %hhd", rwlock.want_write);
	printf_limited("");
	rwspinlock_release_write(&rwlock, flags+3);
	printf_limited("num_readers: %d", rwlock.num_readers);
	printf_limited("want_write: %hhd", rwlock.want_write);
	printf_limited("");
	rwspinlock_acquire_write(&rwlock, flags+4);
	printf_limited("num_readers: %d", rwlock.num_readers);
	printf_limited("want_write: %hhd", rwlock.want_write);
	printf_limited("");

	volatile int b = 0;
	volatile int a = 1 / b;

	assert(a != 1);
	// We're done, just hang...
	hcf();
}
