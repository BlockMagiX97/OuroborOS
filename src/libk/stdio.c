#include <libk/assert.h>
#include <libk/stdio.h>
#include <libk/util.h>
#include <libk/wrap_builtin.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct format_options {
	bool twice_size;
	bool half_size;
	bool quater_size;
	bool zero_pad;
	size_t pad_width;
};
// returns -1 on fail, length of written msg
int conv_ptr(char *dst, size_t maxlenght, const void *conv, const struct format_options *fmt) {
	uint64_t con = (uint64_t)conv;
	size_t width = 2;      // 0x
	size_t num_length = 1; // length of number
	for (size_t i = 1; i < 2 * sizeof(con); i++) {
		if ((con >> (4 * i)) > 0) {
			num_length++;
		}
	}
	width = MAX(width + num_length, fmt->pad_width+2);
	if (maxlenght < width) {
		return -1;
	}
	if (fmt->zero_pad) {
		dst[0] = '0';
		dst[1] = 'x';
		size_t i = 2;
		for (; i < width - num_length; i++)
			dst[i] = '0';
		for (size_t j = 0; j < num_length; j++) {
			uint64_t tmp = (con >> (j * 4)) & 0b1111;
			if (tmp >= 10)
				dst[i + (num_length-(j+1))] = 'a' + (tmp - 10);
			else
				dst[i + (num_length-(j+1))] = '0' + tmp;
		}

	} else {
		size_t i = 0;
		for (; i < width - (num_length + 2); i++)
			dst[i] = ' ';
		dst[i] = '0';
		i++;
		dst[i] = 'x';
		i++;
		for (size_t j = 0; j < num_length; j++) {
			uint64_t tmp = (con >> (j * 4)) & 0b1111;
			if (tmp >= 10)
				dst[i + (num_length-(j+1))] = 'a' + (tmp - 10);
			else
				dst[i + (num_length-(j+1))] = '0' + tmp;
		}
	}
	return width;
};

int conv_x(char *dst, size_t maxlenght, uint64_t conv, const struct format_options *fmt) {
	uint64_t con = conv;
	size_t width = 0;      // 0
	size_t num_length = 1; // length of number
	for (size_t i = 1; i < 2 * sizeof(con); i++) {
		if ((con >> (4 * i)) > 0) {
			num_length++;
		}
	}
	width = MAX(width + num_length, fmt->pad_width);
	if (maxlenght < width) {
		return -1;
	}
	if (fmt->zero_pad) {
		size_t i = 0;
		for (; i < width - num_length; i++)
			dst[i] = '0';
		for (size_t j = 0; j < num_length; j++) {
			uint64_t tmp = (con >> (j * 4)) & 0b1111;
			if (tmp >= 10)
				dst[i + (num_length-(j+1))] = 'a' + (tmp - 10);
			else
				dst[i + (num_length-(j+1))] = '0' + tmp;
		}

	} else {
		size_t i = 0;
		for (; i < width - (num_length); i++)
			dst[i] = ' ';
		for (size_t j = 0; j < num_length; j++) {
			uint64_t tmp = (con >> (j * 4)) & 0b1111;
			if (tmp >= 10)
				dst[i + (num_length-(j+1))] = 'a' + (tmp - 10);
			else
				dst[i + (num_length-(j+1))] = '0' + tmp;
		}
	}
	return width;
};
int conv_d(char *dst, size_t maxlenght, int64_t conv, const struct format_options *fmt) {

	size_t width = 0; // 0
	uint64_t number_part = conv;
	if (conv < 0) {
		number_part = -conv;
		width++;
	}
	size_t num_length = 1;
	// can be smaller but i dont know how many digits has 64 bit number +1
#define NUM_BUF_LEN 64
	char buff[NUM_BUF_LEN] = {0};
	char *buf_ptr = buff+NUM_BUF_LEN-1;

	register uint64_t tmp = 0;
	while ((tmp = (number_part / 10)) > 0) {
		*buf_ptr = '0' + (number_part % 10);
		buf_ptr--;
		number_part = tmp;
		num_length++;
	}
	*buf_ptr = '0' + (number_part % 10);
	width += num_length;
	size_t total_length = MAX(width, fmt->pad_width);
	if (total_length > maxlenght)
		return -1;
	if (fmt->zero_pad) {
		size_t i=0;
		if (conv < 0) {
			dst[i] = '-';
			i++;
		}
		// not width because that includes minus and we already have it
		for (;i<(total_length-num_length);i++) {
			dst[i] = '0';
		}
		size_t j=0;
		for (;i<total_length;i++) {
			dst[i] = buf_ptr[j];
			j++;
		}
	} else {
		size_t i=0;
		for (;i<(total_length-width);i++) {
			dst[i] = ' ';
		}
		if (conv < 0) {
			dst[i] = '-';
			i++;
		}
		size_t j=0;
		for (;i<total_length;i++) {
			dst[i] = buf_ptr[j];
			j++;
		}
	}
	return (int)total_length;
}
int printf_limited(const char *format, ...) {
	char output_buf[INTERNAL_BUFFER_LENGTH] = {0};
	size_t i = 0;
	va_list args;
	va_start(args, format);
	struct format_options fmt = {
	    .twice_size = false, .half_size = false, .quater_size = false, .zero_pad = false, .pad_width = 0};
	bool format_specifier_mode = false;
	bool collecting_fmt_pad_width = false;

#define RESET_FMT_STATE \
	do { \
		fmt.twice_size = false; \
		fmt.half_size = false; \
		fmt.quater_size = false; \
		fmt.zero_pad = false; \
		fmt.pad_width = 0; \
	} while(0)
	
#define RESET_FLAGS_STATE \
	do { \
		format_specifier_mode = false; \
		collecting_fmt_pad_width = false; \
		RESET_FMT_STATE; \
	} while(0)

#define REM_LENGTH (INTERNAL_BUFFER_LENGTH-(i+1))

	// vars used by switches
	int written_bytes;
	const char *string;
	char character;
	for (; *format != '\0'; format++) {
		switch (*format) {
		case '%':
			format_specifier_mode = true;
			break;
		default:
			if (!format_specifier_mode) {
				output_buf[i] = *format;
				i++;
			}
			if (format_specifier_mode) {
				switch (*format) {
				case 'h':
					collecting_fmt_pad_width = false;
					if (fmt.twice_size)
						return -1;
					if (!(fmt.quater_size || fmt.half_size)) {
						fmt.half_size = true;
						break;
					}
					if (fmt.half_size) {
						fmt.half_size = false;
						fmt.quater_size = true;
						break;
					}
					return -1;
				case 'l':
					collecting_fmt_pad_width = false;
					if (fmt.half_size || fmt.quater_size)
						return -1;
					safety_assert(!fmt.twice_size);
					if (!fmt.twice_size) {
						fmt.twice_size = false;
						break;
					}
				case '0':
					if (collecting_fmt_pad_width) {
						fmt.pad_width = fmt.pad_width * 10 + (*format - '0');
						break;
					}
					if (!collecting_fmt_pad_width && !fmt.zero_pad) {
						fmt.zero_pad = true;
						break;
					}
					// doesnt set collecting_fmt_pad_width flag
					return -1;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					if (collecting_fmt_pad_width) {
						fmt.pad_width = fmt.pad_width * 10 + (*format - '0');
						break;
					}
					if (fmt.pad_width == 0) {
						collecting_fmt_pad_width = true;
						fmt.pad_width = (*format - '0');
						break;
					}
					return -1;
				case 'p':
					written_bytes =
					    conv_ptr(output_buf+i, REM_LENGTH, va_arg(args, const void *), &fmt);
					if (written_bytes < 0) {
						return -1;
					}
					i += written_bytes;
					RESET_FLAGS_STATE;
					break;
				case 'x':
					if (fmt.quater_size) {
						// type promotions generate warnings, user should not use with invalid values
						uint8_t val = (va_arg(args, unsigned int));
						written_bytes = conv_x(output_buf+i, REM_LENGTH,val, &fmt);
					} else if (fmt.quater_size) {
						// type promotions generate warnings, user should not use with invalid values
						uint16_t val = (va_arg(args, unsigned int));
						written_bytes = conv_x(output_buf+i, REM_LENGTH,val, &fmt);
					} else if (fmt.twice_size) {
						uint64_t val = (va_arg(args, uint64_t));
						written_bytes = conv_x(output_buf+i, REM_LENGTH,val, &fmt);
					} else {
						uint32_t val = (va_arg(args, uint32_t));
						written_bytes = conv_x(output_buf+i, REM_LENGTH,val, &fmt);
					}
					if (written_bytes < 0) {
						return -1;
					}
					i += written_bytes;
					RESET_FLAGS_STATE;
					break;
				case 'd':
					if (fmt.quater_size) {
						// type promotions generate warnings, user should not use with invalid values
						int8_t val = (va_arg(args, int));
						written_bytes = conv_d(output_buf+i, REM_LENGTH,val, &fmt);
					} else if (fmt.quater_size) {
						// type promotions generate warnings, user should not use with invalid values
						int16_t val = (va_arg(args, int));
						written_bytes = conv_d(output_buf+i, REM_LENGTH,val, &fmt);
					} else if (fmt.twice_size) {
						int64_t val = (va_arg(args, int64_t));
						written_bytes = conv_d(output_buf+i, REM_LENGTH,val, &fmt);
					} else {
						int32_t val = (va_arg(args, int32_t));
						written_bytes = conv_d(output_buf+i, REM_LENGTH,val, &fmt);
					}
					if (written_bytes < 0) {
						return -1;
					}
					i += written_bytes;
					RESET_FLAGS_STATE;
					break;
				case 's':
					string = (va_arg(args, const char *));
					while (*string != '\0') {
						if (i>=REM_LENGTH) {
							return -1;
						}
						output_buf[i] = *string;
						string++;
						i++;
					}
					
					RESET_FLAGS_STATE;
					break;
				case 'c':
					character = (va_arg(args, int));
					output_buf[i] = character;
					i++;
					RESET_FLAGS_STATE;
					break;
				}
			}
			break;
		}
	}
	va_end(args);
	weak_puts(output_buf);
#undef REM_LENGTH
#undef RESET_FLAGS_STATE
#undef RESET_FMT_STATE
	return i;
}
