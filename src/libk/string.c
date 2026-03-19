#include <libk/string.h>
#include <libk/wrap_builtin.h>
#include <limits.h>

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
	uint8_t *restrict pdest = (uint8_t *restrict)dest;
	const uint8_t *restrict psrc = (const uint8_t *restrict)src;

	for (size_t i = 0; i < n; i++) {
		pdest[i] = psrc[i];
	}

	return dest;
}

void *memset(void *s, int c, size_t n) {
	uint8_t *p = (uint8_t *)s;

	for (size_t i = 0; i < n; i++) {
		p[i] = (uint8_t)c;
	}

	return s;
}

void *memmove(void *dest, const void *src, size_t n) {
	uint8_t *pdest = (uint8_t *)dest;
	const uint8_t *psrc = (const uint8_t *)src;

	if (src > dest) {
		for (size_t i = 0; i < n; i++) {
			pdest[i] = psrc[i];
		}
	} else if (src < dest) {
		for (size_t i = n; i > 0; i--) {
			pdest[i - 1] = psrc[i - 1];
		}
	}

	return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	const uint8_t *p1 = (const uint8_t *)s1;
	const uint8_t *p2 = (const uint8_t *)s2;

	for (size_t i = 0; i < n; i++) {
		if (p1[i] != p2[i]) {
			return p1[i] < p2[i] ? -1 : 1;
		}
	}

	return 0;
}
void *memmem(const void *haystack, size_t hsize, const void *needle, size_t nsize) {
	if (nsize == 0) {
		return (void *)haystack;
	}
	if (hsize == 0) {
		return NULL;
	}
	const uint8_t *hay = haystack;
	const uint8_t *need = needle;
	for (size_t hi = 0; hi < hsize; hi++) {
		size_t ni;
		for (ni = 0; ni < nsize && (hi + ni) < hsize; ni++) {
			if (hay[hi + ni] != need[ni]) {
				break;
			}
		}
		if (ni == nsize) {
			return (void *)(hay + hi);
		}
		continue;
	}
	return NULL;
}
void *memchr(const void *s, uint8_t c, size_t n) {
	const uint8_t *mem_reg = s;
	for (size_t i = 0; i < n; i++) {
		if (mem_reg[i] == c) {
			return (void *)(mem_reg + i);
		}
	}
	return NULL;
}
char *strchr(const char *s, int c) {
	while (*s) {
		if (c == *s) {
			// discard const
			return (char *)s;
		}
		s++;
	}
	if (c == '\0') {
		return (char *)s;
	}
	return NULL;
}
size_t strlen(const char *s) {
	size_t i = 0;
	while (s[i])
		i++;
	return i;
}
size_t strnlen(const char *s, size_t maxlen) {
	size_t i = 0;
	while (s[i] && i < maxlen)
		i++;
	return i;
}

// dst might not be null terminated
char* stpncpy(char *dst, const char *src, size_t maxlen) {
	for (size_t i=0;i<maxlen;i++) {
		*dst = src[i];
		if (src[i] == '\0') {
			break;
		}

		dst++;
	}
	return dst;
}

// assumes decimal string n digits long
uint64_t antou64(const char *str, size_t len) {
	if (len <= 0) {
		return 0;
	}
	uint64_t out = 0;
	uint64_t power = 1;
	for (; len-- > 0;) {
		if (UNLIKELY(str[len] - '0' >= 10 || str[len] - '0' < 0)) {
			return UINT64_MAX;
		}
		out += (str[len] - '0') * power;
		power *= 10;
	}
	return out;
}
