#include <libk/assert.h>
#include <libk/string.h>
#include <limine.h>
#include <screen/screen.h>

// clang-format off
__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 0
};
// clang-format on

struct limine_framebuffer *curr_framebuffer;

int init_screen() {
	if (framebuffer_request.response->framebuffer_count == 0) {
		return -1;
	}
	curr_framebuffer = framebuffer_request.response->framebuffers[0];
	return 0;
}

uint64_t normalize_color_8bit(struct screen_color_8bit color) {
	// mask is also max value
	uint32_t red_mask = (1 << curr_framebuffer->red_mask_size) - 1;
	uint32_t green_mask = (1 << curr_framebuffer->green_mask_size) - 1;
	uint32_t blue_mask = (1 << curr_framebuffer->blue_mask_size) - 1;

	uint32_t red_scaled = (color.red * red_mask) / (UINT8_MAX);
	uint32_t green_scaled = (color.green * green_mask) / (UINT8_MAX);
	uint32_t blue_scaled = (color.blue * blue_mask) / (UINT8_MAX);

	uint64_t tmp = 0;
	tmp |= ((red_scaled & red_mask) << curr_framebuffer->red_mask_shift);
	tmp |= ((green_scaled & green_mask) << curr_framebuffer->green_mask_shift);
	tmp |= ((blue_scaled & blue_mask) << curr_framebuffer->blue_mask_shift);
	return tmp;
}

void draw_pixel(struct screen_color_8bit color, struct screen_pos pos) {
	safety_assert_no_print(pos.x < curr_framebuffer->width);
	safety_assert_no_print(pos.y < curr_framebuffer->height);

	assert_no_print(curr_framebuffer->bpp % 8 == 0);
	uint16_t pixelwidth = curr_framebuffer->bpp / 8;
	void *row = curr_framebuffer->address + pos.y * curr_framebuffer->pitch;

	void *pixel = row + pos.x * pixelwidth;

	uint64_t ser_color = normalize_color_8bit(color);

	memcpy(pixel, &ser_color, pixelwidth);
}

void draw_rect(struct screen_color_8bit color, struct screen_pos pos, uint32_t height, uint32_t width) {

	safety_assert_no_print(pos.x + width <= curr_framebuffer->width);
	safety_assert_no_print(pos.y + height <= curr_framebuffer->height);

	assert_no_print(curr_framebuffer->bpp % 8 == 0);
	uint16_t pixelwidth = curr_framebuffer->bpp / 8;

	void *row = curr_framebuffer->address + pos.y * curr_framebuffer->pitch;
	void *where = row + pos.x * pixelwidth;
	uint64_t ser_color = normalize_color_8bit(color);

	for (uint32_t i = 0; i < height; i++) {
		for (uint32_t j = 0; j < width; j++) {
			memcpy(where + j * pixelwidth, &ser_color, pixelwidth);
		}
		where += curr_framebuffer->pitch;
	}
}
