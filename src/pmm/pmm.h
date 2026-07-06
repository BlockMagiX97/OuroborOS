#ifndef OUROBOROS_PMM_PMM_H
#define OUROBOROS_PMM_PMM_H
#include <arch/curr/mem_addr.h>
#include <libk/typedef.h>
typedef uint64_t pfn_t;
#define PFN_INVALID 0


void init_pmm();

paddr_t alloc_zeroed_page();
paddr_t alloc_free_page();
err_t alloc_paddr(paddr_t paddr);

void free_active_page(paddr_t paddr);

enum mem_location {
	UNDER_16M=0,
	UNDER_4G,
	ABOVE_4G,
};
paddr_t alloc_consecutive_in_mem_loc(uint32_t num, enum mem_location location);
#endif
