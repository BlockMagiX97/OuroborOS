#ifndef OUROBOROS_PMM_ARENAS_H
#define OUROBOROS_PMM_ARENAS_H
#include <stdbool.h>
#include <stddef.h>

#include <arch/curr/mem_addr.h>

struct free_block_header {
	paddr_t next;
	// count of continous blocks (used only for init) set to one for normal
	uint64_t size;
};

enum pmm_lock_type {
	PMM_GLOBAL,
	PMM_CPU_LOCAL,
};
enum pmm_arena_type {
	PMM_4K,
	PMM_2M,
	PMM_1G,
	PMM_SPECIAL,
};

struct pmm_arena {
	enum pmm_lock_type lock_type;
	// TODO: add locking system

	enum pmm_arena_type type;
	paddr_t start_paddr;
	paddr_t end_addr;

	paddr_t freelist_head;
};
#endif
