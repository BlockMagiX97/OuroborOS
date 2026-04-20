#ifndef OUROBOROS_PMM_PMM_H
#define OUROBOROS_PMM_PMM_H
#include <stddef.h>

#include <arch/x86_64/mem_addr.h>
#include <arch/x86_64/spinlock.h>

#include <arch/x86_64/vmm/paging.h>

// for use by vmm
paddr_t pmm_alloc(enum page_size size);
void pmm_free(paddr_t addr, enum page_size size);

// returns allocation that must be lower then addr_less_then, continous_page_frames wide, and aligned to at least n page boundery
paddr_t pmm_alloc_constrained(paddr_t addr_less_then, size_t continous_page_frames, unsigned short alignment);
void pmm_free_constrained(paddr_t addr, size_t continous_page_frames);
#endif
