#ifndef OUROBOROS_PMM_PMM_H
#define OUROBOROS_PMM_PMM_H
#include <arch/curr/mem_addr.h>
typedef uint64_t pfn_t;
#define PFN_INVALID 0

void init_pmm();
paddr_t pfn_to_paddr(pfn_t pfn);
pfn_t paddr_to_pfn(paddr_t paddr);

#include <arch/curr/spinlock.h>
enum list_type {
	ZEROED_LIST=0,
	FREE_LIST,
	USED_LIST,
	BAD_LIST
};
enum mem_location  {
	UNDER_16M,
	UNDER_4G,
	ABOVE_4G
};
struct pfd {
	spinlock_t lock;//4
	uint32_t ref_cnt;//4
	struct pfd *next;//8
	union {
		struct {
			uint64_t list_type: 2; // used to see which list is this pfn partof
			uint64_t mem_location: 2; 
			uint64_t reading: 1;
			uint64_t writing: 1;
			uint64_t zone_id: 16;
		} flags;
		uint64_t entire_flags;
	}; // 8
};
#endif
