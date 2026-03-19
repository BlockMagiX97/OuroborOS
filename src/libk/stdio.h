#ifndef OUROBOROS_LIBK_STDIO_H
#define OUROBOROS_LIBK_STDIO_H
#include <stdarg.h>

#define INTERNAL_BUFFER_LENGTH 2048
int printf_limited(const char* format, ...);
#endif
