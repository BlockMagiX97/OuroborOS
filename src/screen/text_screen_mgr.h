#ifndef OUROBOROS_SCREEN_TEXT_SCREEN_MGR_H
#define OUROBOROS_SCREEN_TEXT_SCREEN_MGR_H
#include <libk/ring_queue.h>
#include <screen/screen.h>
#include <stddef.h>
#define LINE_BUFF_SIZE 256

struct text_screen_mgr_line {
	// due to ring buffer design when overwriting if last line in which color wraps to next line would be discarded
	// then we would lose that color
	struct screen_color_8bit line_begin_fg_color;
	struct screen_color_8bit line_begin_bg_color;
	// optimization so we can easily set line_begin colors when adding new lines
	struct screen_color_8bit line_end_fg_color;
	struct screen_color_8bit line_end_bg_color;
	// start of line
	char line_buff[LINE_BUFF_SIZE];
	// points to null terminator
	char *line_end;
};

MAKE_RINGQUEUE_STRUCT_DECL(text_screen_mgr_line)

void weak_puts(const char *str);
uint32_t get_curr_max_line_width();
#endif
