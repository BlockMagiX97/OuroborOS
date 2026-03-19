#ifndef OUROBOROS_SCREEN_SCREEN_H
#define OUROBOROS_SCREEN_SCREEN_H
#include <limine.h>
#include <stdint.h>

struct screen_pos {
	uint32_t x;
	uint32_t y;
};

struct screen_color_8bit {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

extern struct limine_framebuffer *curr_framebuffer;

int init_screen();
uint64_t normalize_color_8bit(struct screen_color_8bit color);
void draw_pixel(struct screen_color_8bit color, struct screen_pos pos);
void draw_rect(struct screen_color_8bit color, struct screen_pos pos, uint32_t height, uint32_t width);
#endif
