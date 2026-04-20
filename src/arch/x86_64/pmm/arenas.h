#ifndef OUROBOROS_PMM_ARENAS_H
#define OUROBOROS_PMM_ARENAS_H
#include <stdbool.h>
#include <stddef.h>

#include <arch/x86_64/mem_addr.h>
#include <arch/x86_64/spinlock.h>
#include <arch/x86_64/vmm/paging.h>

#define PMM_MAX_ARENAS 64

#define IS_FREE_BLOCK_HEADER(block_header) \
	((block_header)->num_used == 0)

struct free_block_header_4k {
	paddr_t next;
	paddr_t prev;
	struct {
		unsigned int count_consecutive; // only used for init ()
	} free;
};
struct free_block_header_2m {
	// since this is stored in the first page it must not conflict with 4K header
	struct free_block_header_4k header_4k;

	paddr_t next;
	paddr_t prev;

	struct {
		unsigned int count_consecutive; // only used for init ()
	} free;
	struct {
		uint16_t num_used;
		paddr_t freelist;
	} partial;
};
struct free_block_header_1g {
	// since this is stored in the first page it must not conflict with 4K header and 2M header
	struct free_block_header_2m header_2m;

	paddr_t next;
	paddr_t prev;

	struct {
		unsigned int count_consecutive; // only used for init ()
	} free;
	struct {
		uint16_t num_used;

		paddr_t freelist;
		paddr_t partiallist; 
		paddr_t partial_full_list;
	} partial;
};
_Static_assert(sizeof(struct free_block_header_1g) <= PAGE_FRAME_SIZE, "Largest free_block_header surpassed PAGE_FRAME_SIZE make it smaller");

enum ppm_arena_type {
	PMM_ARENA_NORMAL,
	PMM_ARENA_CONSTRAINED,
};
struct pmm_arena {
	spinlock_t lock;

	enum ppm_arena_type type;
	union {
		struct {
			bool has_4k;
			bool has_2m;
			bool has_1g;
			paddr_t freelist_4k;
			paddr_t freelist_2m;
			paddr_t freelist_1g;

			paddr_t partiallist_2m;
			paddr_t partiallist_full_2m;
			paddr_t partiallist_1g;
			paddr_t partiallist_full_1g;
		} norm;
		struct {
			unsigned int num_pages;
			paddr_t bitmap_addr;
		} constrain;
	};
};

struct pmm_arena_collector {
	rwspinlock_t lock;

	unsigned int num_normal_not_full_arenas;
	struct pmm_arena *normal_not_full_arenas[PMM_MAX_ARENAS];

	struct pmm_arena all_arenas[PMM_MAX_ARENAS];
	paddr_t arena_starts[PMM_MAX_ARENAS]; // at i it contains all_arrenas[i] start address
	paddr_t arena_ends[PMM_MAX_ARENAS]; // at i it contains all_arrenas[i] end address
};
extern struct pmm_arena_collector pmm_main;

#endif
