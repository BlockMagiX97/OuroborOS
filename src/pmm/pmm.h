#ifndef OUROBOROS_PMM_PMM_H
#define OUROBOROS_PMM_PMM_H
#include <stddef.h>

#include <arch/curr/mem_addr.h>
#include <vmm/paging.h>
#include <pmm/arenas.h>

#define PMM_MAX_ARENAS 256
struct pmm_arena_collector {
	uint32_t num_total_arenas;
	uint32_t num_global_arenas;

	struct pmm_arena* global_arenas[256];
	struct pmm_arena* all_arenas[256];
	// TODO: CPU local arenas
	
};
extern struct pmm_arena_collector pmm_main;

// for use by vmm
paddr_t pmm_alloc(enum page_size size);
// returns allocation that must be lower then addr_less_then, continous_page_frames wide, and aligned to at least (1<<alignment_shift)  boundery
paddr_t pmm_alloc_constrained(paddr_t addr_less_then, size_t continous_page_frames, uint8_t alignment_shift);
void pmm_free(paddr_t addr, size_t continous_page_frames);
#endif
