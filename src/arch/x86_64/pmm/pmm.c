#include <arch/x86_64/kpanic.h>
#include <arch/x86_64/mem_addr.h>
#include <arch/x86_64/spinlock.h>

#include <libk/assert.h>

#include <arch/x86_64/hhdm.h>
#include <arch/x86_64/pmm/arenas.h>
#include <arch/x86_64/vmm/paging.h>

#include <arch/x86_64/pmm/pmm.h>

#define ALIGN_DOWN(addr, shift) \
	((addr) & (~((1 << (shift))-1)))
#define ALIGN_UP(addr, shift) \
	(ALIGN_DOWN(addr, shift)+(1<<shift))


#define SHIFT_4KIB 12
#define SHIFT_2M 21
#define SHIFT_1G 30
static bool is_in_partial_chunk_4k(paddr_t arena_start, paddr_t arena_end, paddr_t addr) {
	if (arena_start < ALIGN_DOWN(addr, SHIFT_2M)) {
		return false;
	}
	if (arena_end < ALIGN_UP(addr, SHIFT_2M)) {
		return false;
	}
	return true;
}
static bool is_in_partial_chunk_2m(paddr_t arena_start, paddr_t arena_end, paddr_t addr) {
	if (arena_start < ALIGN_DOWN(addr, SHIFT_1G)) {
		return false;
	}
	if (arena_end < ALIGN_UP(addr, SHIFT_1G)) {
		return false;
	}
	return true;
}
enum pmm_block_size {
	PMM_4K,
	PMM_2M,
	PMM_1G,
};
paddr_t pmm_alloc(enum page_size size) {
	uint64_t flags;
	paddr_t alloc_addr;

	struct pmm_arena *working_arena;
	enum pmm_block_size type;
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
		kpanic("PMM Invalid size recieved");
	}

	// TODO: add cpu local arena support

	while (true) {
		// to not starve this lock we need to release in a loop (possible to have this lock acquired while
		// someone wants to add to normal_not_full_arenas a new arena that could satisfy our request
		rwspinlock_acquire_read(&pmm_main.lock, &flags);
		for (unsigned int i = 0; i < pmm_main.num_normal_not_full_arenas; i++) {
			if (spinlock_try_acquire(&(pmm_main.normal_not_full_arenas[i]->lock), NULL)) {
				working_arena = pmm_main.normal_not_full_arenas[i];
				switch (type) {
				case PMM_4K:
					if (working_arena->norm.has_4k) {
						goto found_working_arena;
					}
					__attribute((fallthrough));
				case PMM_2M:
					if (working_arena->norm.has_2m) {
						goto found_working_arena;
					}
					__attribute((fallthrough));
				case PMM_1G:
					if (working_arena->norm.has_1g) {
						goto found_working_arena;
					}
					break;
				}
				// is full, but wasn't updated yet, release lock and continue
				spinlock_release(&(pmm_main.normal_not_full_arenas[i]->lock), NULL);
			}
		}
		rwspinlock_release_read(&pmm_main.lock, &flags);
	};
// pmm_main must be reader
// arena must be locked
found_working_arena:
	struct free_block_header_1g *header_1g;
	struct free_block_header_2m *header_2m;
	struct free_block_header_2m *header_2m_tmp;
	paddr_t tmp_paddr;

#define RET_NO_MEM(condition) \
if (condition) { \
	alloc_addr = PADDR_INVALID; \
	goto ret_release_pmm_lock; \
}
	switch (type) {


	case PMM_1G:
		alloc_addr = working_arena->norm.freelist_1g;
		RET_NO_MEM(alloc_addr == PADDR_INVALID)
		header_1g = (void *)hhdm_phys_to_virt(alloc_addr);
		working_arena->norm.freelist_1g = header_1g->next;
		if (working_arena->norm.freelist_1g == PADDR_INVALID) {
			working_arena->norm.has_1g = false;
			break;
		}
		header_1g = (void *)hhdm_phys_to_virt(header_1g->next);
		header_1g->prev = PADDR_INVALID;
		break;
	case PMM_2M:
		alloc_addr = working_arena->norm.freelist_2m;
		if (alloc_addr == PADDR_INVALID) {
			// no in freelist try partiallist_1g
			RET_NO_MEM(working_arena->norm.partiallist_1g == PADDR_INVALID)
			header_1g = (void *)hhdm_phys_to_virt(working_arena->norm.partiallist_1g);
			RET_NO_MEM(header_1g->partial.freelist == PADDR_INVALID)

			alloc_addr = header_1g->partial.freelist;
			header_2m = (void *)hhdm_phys_to_virt(alloc_addr);
			header_1g->partial.freelist = header_2m->next;
			if (header_1g->partial.freelist == PADDR_INVALID) {
				working_arena->norm.has_1g = false;
				break;
			}
			header_2m = (void *)hhdm_phys_to_virt(header_2m->next);
			header_2m->prev = PADDR_INVALID;
			break;
		} else {
			header_2m = (void *)hhdm_phys_to_virt(alloc_addr);
			working_arena->norm.freelist_2m = header_2m->next;
			if (working_arena->norm.freelist_2m == PADDR_INVALID) {
				if (working_arena->norm.partiallist_1g == PADDR_INVALID && working_arena->constrain) {
					working_arena->norm.has_2m = false;
				}
				break;
			}
			header_2m = (void *)hhdm_phys_to_virt(header_2m->next);
			header_2m->prev = PADDR_INVALID;
			break;
		}

		break;
	case PMM_4K:
		break;
	}


#undef RET_NO_MEM
ret_release_pmm_lock:
	rwspinlock_release_read(&pmm_main.lock, &flags);
	return alloc_addr;
};
void pmm_free(paddr_t addr, enum page_size size) {
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
	uint64_t flags;
	rwspinlock_acquire_read(&pmm_main.lock, &flags);
	for (uint32_t i = 0; i < pmm_main.num_total_arenas; i++) {
		switch (pmm_main.all_arenas[i]->lock_type) {
		case PMM_GLOBAL:
			break;
		default:
			kpanic("NOT SUPPORTED ARENA TYPE");
		}
	}
}
paddr_t pmm_alloc_constrained(paddr_t addr_less_then, size_t continous_page_frames, uint8_t alignment_shift);
paddr_t pmm_free_constrained(paddr_t addr_less_then, size_t continous_page_frames, uint8_t alignment_shift);
