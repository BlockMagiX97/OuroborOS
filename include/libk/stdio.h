#ifndef OUROBOROS_LIBK_STDIO_H
#define OUROBOROS_LIBK_STDIO_H
#include <stdarg.h>

#define INTERNAL_BUFFER_LENGTH 2048
int printf_limited(const char* format, ...);
int printf_limited_custom_func(void (*send_buffer_func)(const char*), const char *format, ...);
#endif
