#ifndef OUROBOROS_LIBK_STRING_H
#define OUROBOROS_LIBK_STRING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
void *memcpy(void *restrict dest, const void *restrict src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memmem(const void *haystack, size_t hsize, const void *needle, size_t nsize);
void *memchr(const void *s, uint8_t c, size_t n);

size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);
char* stpncpy(char *dst, const char *src, size_t maxlen);
char *strchr(const char *s, int c);

uint64_t antou64(const char *str, size_t len);

#endif
