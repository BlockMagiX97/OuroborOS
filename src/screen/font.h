#ifndef OUROBOROS_SCREEN_FONT_H
#define OUROBOROS_SCREEN_FONT_H
#include <screen/screen.h>
#include <stdint.h>

void draw_char(char ch, struct screen_color_8bit fg_color, struct screen_color_8bit bg_color, struct screen_pos pos,
	       uint32_t scale);
uint32_t draw_string(const char *string, struct screen_color_8bit fg_color, struct screen_color_8bit bg_color,
		     struct screen_pos pos, uint32_t scale);
#endif
