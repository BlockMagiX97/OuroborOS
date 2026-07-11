#ifndef OUROBOROS_ARCH_X86_64_MEM_ADDR_H
#define OUROBOROS_ARCH_X86_64_MEM_ADDR_H
#include <limits.h>
#include <stdint.h>
typedef uint64_t paddr_t;
#define PADDR_MAX UINT64_MAX
#define PADDR_INVALID PADDR_MAX
typedef void *vaddr_t;
#define VADDR_MAX UINT64_MAX

#endif
