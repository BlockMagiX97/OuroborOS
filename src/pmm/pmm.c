#include <arch/curr/kpanic.h>
#include <pmm/arenas.h>
#include <pmm/pmm.h>

struct pmm_arena_collector pmm_main;
paddr_t pmm_alloc(enum page_size size) {
	// try to find cpu local
	// TODO: implement it

	struct pmm_arena* working_arena;
	enum pmm_arena_type type;
	switch (size) {
	case PS_4KiB:
		type = PMM_4K;
		break;
	case PS_2MiB:
		type = PMM_2M;
		break;
	case PS_1GiB:
		type = PMM_1G;
		break;
	default:
		kpanic("Invalid size");
	}
	// no cpu local, search globals
	for (uint32_t i = 0; i < pmm_main.num_global_arenas; i++) {
		if (pmm_main.global_arenas[i]->type == type) {
			// TODO: add logic for aquiring lock and skipping locked ones
		}
	}
};
paddr_t pmm_alloc_constrained(paddr_t addr_less_then, size_t continous_page_frames, uint8_t alignment_shift);
void pmm_free(paddr_t addr, size_t continous_page_frames);
