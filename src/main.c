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

#include <gdt.h>
#include <limine.h>
#include <mem.h>

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

	init_hhdm();
	init_gdt(); // finally done it was a pain
	if (init_screen() < 0) {
		hcf();
	}

	struct screen_color_8bit color = {.red = 0, .green = 255, .blue = 0};
	struct screen_pos pos = {.x = 0, .y = 0};
	draw_rect(color, pos, curr_framebuffer->height, curr_framebuffer->width);

	printf_limited("curr_framebuffer addr: %p", curr_framebuffer->address);
	printf_limited("addr: %p", curr_framebuffer->address);
	printf_limited("addr0padded: %010p", (void*)17);
	printf_limited("addr padded: %10p", (void*)11);
	printf_limited("overflow_byte: %hhx", 256);
	printf_limited("byte : %hhx", -122);
	printf_limited("byte0padded: %010hhx", -122);
	printf_limited("byte padded: %10hhx", -122);
	printf_limited("d: %hhd", -122);
	printf_limited("d0padded: %010hhd", -122);
	printf_limited("d padded: %10hhd", -122);
	printf_limited("string: %s", "printf is so fucking complex");
	printf_limited("char: %c", 'c');
	printf_limited("");
	printf_limited("testing multiple prints: %016ld;%c%hhd;%s;%016p;%010hx\n\nhello", 0x1337, 'c', 4,"test multi\nline", &limine_base_revision, 16000);
	printf_limited("testing multiple prints: %016ld;%c%hhd;%s;%016p;%010hx\n\nhello", 0x1337, 'c', 4,"test multi\nline", &limine_base_revision, 16000);
	assert(1 != 1);
	// We're done, just hang...
	hcf();
}
