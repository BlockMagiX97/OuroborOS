#include <screen/text_screen_mgr.h>

#include <libk/assert.h>
#include <libk/ctype.h>
#include <libk/ring_queue.h>
#include <libk/string.h>
#include <libk/util.h>
#include <screen/font.h>
#include <screen/screen.h>
#include <stdint.h>

const uint32_t text_screen_mgr_scale = 1;
struct text_screen_mgr_line __main_text_ringq_buffer[512];
static RINGQUEUE_STRUCT(text_screen_mgr_line) main_text_ringq = RINGQUEUE_INIT(__main_text_ringq_buffer, 512);

struct screen_color_8bit default_fg_color = {.red = 255, .green = 255, .blue = 255};
struct screen_color_8bit default_bg_color = {.red = 0, .green = 0, .blue = 0};
struct screen_color_8bit default_terminal_color = {.red = 0, .green = 0, .blue = 0};

size_t num_digit(const char *str, size_t max_len) {
	size_t num_dig = 0;
	while (num_dig < max_len && isdigit(str[num_dig]))
		num_dig++;
	return num_dig;
}

char *parse_true_color(const char *str, size_t max_len, struct screen_color_8bit *out) {
	const char *end_ptr = memchr(str, ';', max_len);
	assert_no_print(end_ptr != NULL);
	size_t num_len = end_ptr - str;
	uint64_t must_be_2 = antou64(str, num_len);
	assert_no_print(must_be_2 == 2);

	max_len -= num_len + 1;
	str = end_ptr + 1;
	end_ptr = memchr(str, ';', max_len);
	assert_no_print(end_ptr != NULL);
	num_len = end_ptr - str;
	uint64_t red = antou64(str, num_len);
	assert_no_print(red <= 255);

	max_len -= num_len + 1;
	str = end_ptr + 1;
	end_ptr = memchr(str, ';', max_len);
	assert_no_print(end_ptr != NULL);
	num_len = end_ptr - str;
	uint64_t green = antou64(str, num_len);
	assert_no_print(green <= 255);

	max_len -= num_len + 1;
	str = end_ptr + 1;
	end_ptr = memchr(str, 'm', max_len);
	assert_no_print(end_ptr != NULL);
	num_len = end_ptr - str;
	uint64_t blue = antou64(str, num_len);
	assert_no_print(blue <= 255);

	out->red = red;
	out->green = green;
	out->blue = blue;
	return (char *)end_ptr;
}

// currently supports only color escape codes and in limited matter
// return ptr to end byte in escape seq
char *parse_escape_sequence(const char *str, size_t max_len, struct screen_color_8bit *fg, struct screen_color_8bit *bg,
			    bool *set_fg, bool *set_bg) {
	safety_assert_no_print(max_len > 2);
	assert_no_print(str[0] == '\033');
	assert_no_print(str[1] == '[');

	max_len -= 2;
	str += 2;

	size_t num_digits = num_digit(str, max_len);
	uint64_t mode = 0;
	if (num_digits > 0)
		mode = antou64(str, num_digits);
	safety_assert_no_print(max_len > num_digits);
	char end_char = str[num_digits];
	max_len -= num_digits;
	str += num_digits;
	struct screen_color_8bit *color;
	const char *end_ptr = str;
	switch (end_char) {
	case 'm':
		switch (mode) {
		case 0:
			*fg = default_fg_color;
			*bg = default_bg_color;
			if (set_fg != NULL)
				*set_fg = true;
			if (set_bg != NULL)
				*set_bg = true;
			break;
		default:
			hcf();
		}
		break;
	case ';':
		color = bg;
		switch (mode) {
		case 38:
			color = fg;
			if (set_fg != NULL)
				*set_fg = true;
			__attribute__((fallthrough));
		case 48:
			end_ptr = parse_true_color(str+1, max_len-1, color);
			if (set_bg != NULL)
				*set_bg = true;
			break;
		default:
			hcf();
		}
		break;

	default:
		hcf();
	}
	return (char *)end_ptr;
}
uint32_t get_max_line_width() {
	uint32_t num_ch_p_l = curr_framebuffer->width / 8 / text_screen_mgr_scale;
	return num_ch_p_l;
}
void text_screen_draw(RINGQUEUE_STRUCT(text_screen_mgr_line) * text, uint32_t scale) {
	const uint32_t num_lines = curr_framebuffer->height / 8 / scale;
	const uint32_t num_ch_per_line = curr_framebuffer->width / 8 / scale;

	uint32_t min_lines = MIN(text->cap - text->neg_len, num_lines);
	struct screen_pos pos = {.x = 0, .y = 0};
	for (uint32_t i = 0; i < min_lines; i++) {
		struct text_screen_mgr_line *line = &(RINGQUEUE_I_TAIL(text, min_lines - 1 - i));
		struct screen_color_8bit curr_fg_color = line->line_begin_fg_color;
		struct screen_color_8bit curr_bg_color = line->line_begin_bg_color;

		uint32_t num_char_printed = 0;
		pos.x = 0;
		for (const char *ptr = line->line_buff; num_char_printed < num_ch_per_line && ptr < line->line_end;
		     ptr++) {
			if (*ptr == '\033') {
				ptr = parse_escape_sequence(ptr, line->line_end - ptr, &curr_fg_color, &curr_bg_color,
							    NULL, NULL);
				continue;
			}

// make warning disappear (condition is always true(but check is included for clarity))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
			if (*ptr < 128 && *ptr > 0) {
#pragma GCC diagnostic pop
				num_char_printed++;
				draw_char(*ptr, curr_fg_color, curr_bg_color, pos, scale);
				pos.x += 8 * scale;
			}
		}
		draw_rect(default_terminal_color, pos, 8 * scale, curr_framebuffer->width - pos.x);
		pos.y += 8 * scale;
	}
}
void add_line(RINGQUEUE_STRUCT(text_screen_mgr_line) * text, const char *line, size_t length) {
	struct text_screen_mgr_line mgr_line;

	stpncpy(mgr_line.line_buff, line, MIN(length, LINE_BUFF_SIZE - 1));
	mgr_line.line_end = ((char *)mgr_line.line_buff) + (MIN(length, LINE_BUFF_SIZE - 1));
	*(mgr_line.line_end) = '\0';
	if (RINGQUEUE_IS_EMPTY(text)) {
		mgr_line.line_begin_fg_color = default_fg_color;
		mgr_line.line_begin_bg_color = default_bg_color;
	} else {
		mgr_line.line_begin_fg_color = RINGQUEUE_I_TAIL(text, 0).line_end_fg_color;
		mgr_line.line_begin_bg_color = RINGQUEUE_I_TAIL(text, 0).line_end_bg_color;
	}

	const char *ptr = mgr_line.line_end;
	bool has_bg = false;
	bool has_fg = false;
	for (; ptr >= mgr_line.line_buff; ptr--) {
		if (*ptr == '\033') {
			parse_escape_sequence(ptr, mgr_line.line_end - ptr, &mgr_line.line_end_fg_color,
					      &mgr_line.line_end_bg_color, &has_fg, &has_bg);
			if (has_fg && has_bg) {
				break;
			}
		}
	}
	if (!has_fg)
		mgr_line.line_end_fg_color = mgr_line.line_begin_fg_color;
	if (!has_bg)
		mgr_line.line_end_bg_color = mgr_line.line_begin_bg_color;

	RINGQUEUE_PUSH(text, &mgr_line);
	RINGQUEUE_I_TAIL(text, 0).line_end = ((char *)RINGQUEUE_I_TAIL(text, 0).line_buff) + (MIN(length, LINE_BUFF_SIZE - 1));
}
void add_line_wrapping(RINGQUEUE_STRUCT(text_screen_mgr_line) * text, const char *line, size_t length) {
	struct screen_color_8bit useless;
	size_t num_printable_ch = 0;
	for (size_t i = 0; i < length; i++) {
		if (line[i] == '\033') {
			i = parse_escape_sequence(line + i, length - i, &useless, &useless, NULL, NULL) - line;
		} else if (line[i] < 128 && line[i] > 0) {
			num_printable_ch++;
		}
		if (num_printable_ch >= get_max_line_width()) {
			add_line(text, line, i + 1);
			line += i + 1;
			length = length - (i + 1);
			i = 0;
			num_printable_ch = 0;
		}
	}
	add_line(text, line, length);
}
void weak_puts(const char *str) {
	size_t length = strlen(str);
	const char *ptr = str;
	const char *next_ptr = str;

	while (1) {
		next_ptr = memchr(ptr, '\n', length);
		if (next_ptr == NULL) {
			add_line_wrapping(&main_text_ringq, ptr, length);
		}
		// next_ptr is always bigger
		if (length < (size_t)((next_ptr - ptr) + 1)) {
			break;
		}
		add_line_wrapping(&main_text_ringq, ptr, (size_t)((next_ptr - ptr)));
		length -= (size_t)((next_ptr - ptr) + 1);
		ptr = next_ptr + 1;
	}
	text_screen_draw(&main_text_ringq, text_screen_mgr_scale);
}
