#include <stdarg.h>

#include <limine.h>

#include <libk/string.h>
#include <libk/stdio.h>
#include <libk/wrap_builtin.h>

#include <arch/curr/hhdm.h>
#include <arch/curr/kpanic.h>
#include <arch/curr/spinlock.h>

#include <vmm/paging.h>

#include <pmm/pmm.h>

// list_type 0 is default one and order matters (sooner it is the higher priority (bad overwrites reserved)), UNKNOWN is
// exception and has lowest priority
enum pfd_type { UNKNOWN = 0, BAD, RESERVED, RESERVED_RECLAIMABLE, ACTIVE, FREE, ZEROED };
enum list_type { FREE_LIST = 0, ZEROED_LIST };
#define LAST_LIST (ZEROED_LIST)
#define IS_TYPE_AVAL(pfd_type) \
	((pfd_type) == ZEROED || (pdf_type) == FREE)

static paddr_t pfn_to_paddr(pfn_t pfn) {
	if (pfn == PFN_INVALID) {
		return PADDR_INVALID;
	}
	return pfn << PAGE_FRAME_SHIFT;
}
static pfn_t paddr_to_pfn(paddr_t paddr) {
	if (paddr == PADDR_INVALID) {
		return PFN_INVALID;
	}
	return paddr >> PAGE_FRAME_SHIFT;
}

struct list_head {
	pfn_t pfn;
	spinlock_t lock;
};
struct list_head list_heads[LAST_LIST + 1];

static struct list_head *pfd_type_to_list(enum pfd_type pfd_type) {
	switch (pfd_type) {
	case FREE:
		return list_heads + FREE_LIST;
	case ZEROED:
		return list_heads + ZEROED_LIST;
	case UNKNOWN:
		__attribute__((fallthrough));
	case RESERVED:
		__attribute__((fallthrough));
	case RESERVED_RECLAIMABLE:
		__attribute__((fallthrough));
	case ACTIVE:
		__attribute__((fallthrough));
	case BAD:
		return NULL;
	}
	UNREACHABLE;
	return NULL;
};

struct pfd {
	union {
		struct {
			uint32_t type : 3; // enum pfn_type
		};
		uint32_t entire_flags;
	} flags; // 4
	union {
		struct {
			pfn_t next; // 8
			pfn_t prev; // 8
		} list;
		struct {
			spinlock_t lock;  // 4
			uint32_t ref_cnt; // 4
					  // todo add things for paging
		} active;
	};
	// 0x16
} __attribute__((packed));

struct pfd *pfndb;
struct pfd *get_pfd(pfn_t pfn) {
	return pfndb+pfn;
}

// lock the list yourself
static pfn_t pop_list(struct list_head *lh) {
	if (lh->pfn == PFN_INVALID) {
		return PFN_INVALID;
	}
	struct pfd *curr_pfd = get_pfd(lh->pfn);
	pfn_t ret_pfn = lh->pfn;
	struct pfd *next_pfd = get_pfd(curr_pfd->list.next);
	next_pfd->list.prev = PFN_INVALID;
	lh->pfn = curr_pfd->list.next;
	return ret_pfn;
}
// lock the list yourself
static void push_list(struct list_head *lh, pfn_t pfn) {
	struct pfd *curr_pfd = get_pfd(pfn);
	struct pfd *next_pfd = get_pfd(lh->pfn);
	next_pfd->list.prev = pfn;
	curr_pfd->list.next = lh->pfn;
	curr_pfd->list.prev = PADDR_INVALID;
	lh->pfn = pfn;
}

// clang-format off
__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request mem_map_req = {
	.id = LIMINE_MEMMAP_REQUEST_ID,
	.revision = 0
};
// clang-format on

#define MEMMAP_NEEDS_PFD(limine_mmap_type) \
	((limine_mmap_type) != LIMINE_MEMMAP_BAD_MEMORY && (limine_mmap_type) != LIMINE_MEMMAP_RESERVED && (limine_mmap_type) != LIMINE_MEMMAP_RESERVED_MAPPED)

void init_pmm() {
	// init list_heads
	for (int i = 0; i <= LAST_LIST; i++) {
		list_heads[i].pfn = PFN_INVALID;
		list_heads[i].lock = SPINLOCK_INIT_UNLOCKED;
	}

	paddr_t highest_usable_addr = PADDR_INVALID;
	// get minimum needed size
	uint64_t last_backed_entry;
	for (uint64_t i = mem_map_req.response->entry_count; i-- > 0;) {
		struct limine_memmap_entry *entry = mem_map_req.response->entries[i];
		if (MEMMAP_NEEDS_PFD(entry->type)) {
			highest_usable_addr = entry->base + entry->length;
			last_backed_entry = i;
			break;
		}
	}
	uint64_t num_pfds = ALIGN_UP_PAGE_FRAME(highest_usable_addr) >> PAGE_FRAME_SHIFT;
	uint64_t size = num_pfds * sizeof(struct pfd);

	// get backing (from up to not needlessly occupy under 16M and under 4G space)
	uint64_t backing_size;
	paddr_t backing_base;
	for (uint64_t i = mem_map_req.response->entry_count; i-- > 0;) {
		struct limine_memmap_entry *entry = mem_map_req.response->entries[i];
		if (entry->type == LIMINE_MEMMAP_USABLE) {
			if (entry->length >= size) {
				// align it to page boundery for ease of access
				backing_size = ALIGN_UP_PAGE_FRAME(size);
				backing_base = entry->base + (entry->length - backing_size);
				goto found_backing;
			}
		}
	};
	kpanic("NOT ENOUGH MEMORY", "Unable to find backing for pfndb");
found_backing:

	pfndb = (void *)hhdm_phys_to_virt(backing_base);
	// init pfndb
	memset(pfndb, 0, size);

	for (uint64_t i = 0; i < last_backed_entry; i++) {
		struct limine_memmap_entry *entry = mem_map_req.response->entries[i];

#define FOR_LOOP_ENTRY_LENGHT \
	for (uint64_t offset = 0; offset < ALIGN_UP_PAGE_FRAME(entry->length); offset += PAGE_FRAME_SIZE)

#define FOR_ENTRY_LENGTH_SET_IF_HIGHER_PRIORITY(__type) \
	FOR_LOOP_ENTRY_LENGHT { \
		pfn_t curr_pfn = paddr_to_pfn(entry->base + offset); \
		struct pfd *curr_pfd = get_pfd(curr_pfn); \
		if (curr_pfd->flags.type == UNKNOWN || curr_pfd->flags.type > (__type)) { \
			curr_pfd->flags.type = (__type); \
		} \
	}

		switch (entry->type) {
		case LIMINE_MEMMAP_USABLE:
			// this has guaranteed alignment and no overlap
			FOR_LOOP_ENTRY_LENGHT {
				pfn_t curr_pfn = paddr_to_pfn(entry->base + offset);
				struct pfd *curr_pfd = get_pfd(curr_pfn);

				curr_pfd->flags.type = FREE;
				struct list_head *lh = pfd_type_to_list(FREE);
				push_list(lh, curr_pfn);
			}
			break;
		case LIMINE_MEMMAP_RESERVED:
			FOR_ENTRY_LENGTH_SET_IF_HIGHER_PRIORITY(RESERVED);
			break;
		case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
			// even though we can reclaim it, no overlap is not guaranteed, so putting it inside any list
			// could destroy the list
			FOR_ENTRY_LENGTH_SET_IF_HIGHER_PRIORITY(RESERVED);
			break;
		case LIMINE_MEMMAP_ACPI_NVS:
			FOR_ENTRY_LENGTH_SET_IF_HIGHER_PRIORITY(RESERVED);
			break;
		case LIMINE_MEMMAP_BAD_MEMORY:
			FOR_ENTRY_LENGTH_SET_IF_HIGHER_PRIORITY(BAD);
			break;
		case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
			// this has guaranteed alignment and no overlap
			FOR_ENTRY_LENGTH_SET_IF_HIGHER_PRIORITY(RESERVED_RECLAIMABLE);
			break;
		case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
			FOR_ENTRY_LENGTH_SET_IF_HIGHER_PRIORITY(RESERVED);
			break;
		case LIMINE_MEMMAP_FRAMEBUFFER:
			FOR_ENTRY_LENGTH_SET_IF_HIGHER_PRIORITY(RESERVED);
			break;
		case LIMINE_MEMMAP_RESERVED_MAPPED:
			FOR_ENTRY_LENGTH_SET_IF_HIGHER_PRIORITY(RESERVED);
			break;
		}
#undef FOR_ENTRY_LENGTH_SET_IF_HIGHER_PRIORITY
#undef FOR_LOOP_ENTRY_LENGHT
	}
	// reserve backing_space
	for (uint64_t offset = 0; offset < backing_size; offset += PAGE_FRAME_SIZE) {
		pfn_t curr_pfn = paddr_to_pfn(backing_base + offset);
		struct pfd *curr_pfd = get_pfd(curr_pfn);

		if (curr_pfd->list.next != PFN_INVALID) {
			struct pfd *next_pfd = get_pfd(curr_pfd->list.next);
			next_pfd->list.prev = curr_pfd->list.prev;
		}
		if (curr_pfd->list.prev != PFN_INVALID) {
			struct pfd *prev_pfd = get_pfd(curr_pfd->list.prev);
			prev_pfd->list.next = curr_pfd->list.next;
		} else {
			// it is head, update the list head
			pfd_type_to_list(curr_pfd->flags.type)->pfn = curr_pfd->list.next;
		}
		// it is now unlinked
		//
		curr_pfd->flags.type = ACTIVE;
		curr_pfd->active.ref_cnt = 1;
		curr_pfd->active.lock = SPINLOCK_INIT_UNLOCKED;
	}
};


// what list to try in which order
static paddr_t alloc_page(enum list_type *out_list, unsigned int num_lists, ...) {
	pfn_t ret_pfn = PFN_INVALID;
	flags_t flags;
	va_list ap;
	va_start(ap, num_lists);
	for (unsigned int i = 0; i < num_lists; i++) {
		enum list_type list_type = va_arg(ap, enum list_type);
		struct list_head *lh = list_heads + list_type;
		spinlock_acquire(&lh->lock, &flags);
		ret_pfn = pop_list(lh);

		if (ret_pfn != PFN_INVALID) {
			struct pfd *pfd = get_pfd(ret_pfn);

			pfd->active.lock = SPINLOCK_INIT_LOCKED;
			pfd->flags.type = ACTIVE;
			pfd->active.ref_cnt = 1;
			spinlock_release(&pfd->active.lock, NULL);

			spinlock_release(&lh->lock, &flags);
			*out_list = list_type;
			break;
		}
		spinlock_release(&lh->lock, &flags);
	}
	va_end(ap);
	return pfn_to_paddr(ret_pfn);
}
static void zero_paddr_page_frame(paddr_t paddr) {
	memset((void*)hhdm_phys_to_virt(paddr), 0, PAGE_FRAME_SIZE);
}
paddr_t alloc_zeroed_page() {
	enum list_type type;
	paddr_t ret = alloc_page(&type, 2, ZEROED_LIST, FREE_LIST);
	if (ret == PADDR_INVALID) {
		return PADDR_INVALID;
	}
	if (type != ZEROED_LIST) {
		zero_paddr_page_frame(ret);
	}
	return ret;
}
paddr_t alloc_free_page() {
	enum list_type type;
	paddr_t ret = alloc_page(&type, 2, FREE_LIST, ZEROED_LIST);
	if (ret == PADDR_INVALID) {
		return PADDR_INVALID;
	}
	return ret;
};
void free_active_page(paddr_t paddr) {
	pfn_t pfn = paddr_to_pfn(paddr);
	flags_t flags;
	struct pfd *curr_pfd = get_pfd(pfn);
	spinlock_acquire(&curr_pfd->active.lock, &flags);

	// basic double free protection, dont use ref_cnt it is overwriten for other types
	if (curr_pfd->flags.type != ACTIVE) {
		kpanic("DOUBLE FREE DETECTED", "Attempted to free a physical page that isnt active, most probably "
					       "double free!!! May not be code that calls this function");
	}
	if (curr_pfd->active.ref_cnt <= 0) {
		kpanic("DATA CORRUPTION DETECTED", "PFD with type active but ref_cnt less or equal zero found.");
	}

	curr_pfd->active.ref_cnt--;
	if (curr_pfd->active.ref_cnt == 0) {

		struct list_head *lh = list_heads + FREE_LIST;
		spinlock_acquire(&lh->lock, NULL);
		curr_pfd->flags.type = FREE;
		// this is theoreticaly not needed since this lock becomes inactive after FREE is set
		spinlock_release(&curr_pfd->active.lock, NULL);
		push_list(lh, pfn);
		spinlock_release(&lh->lock, &flags);
		return;
	}
	spinlock_release(&curr_pfd->active.lock, &flags);
	return;
}

err_t alloc_paddr(paddr_t paddr) {
	pfn_t pfn = paddr_to_pfn(paddr);
	flags_t flags;
	struct pfd *curr_pfd = get_pfd(pfn);
	(void)flags;
	(void)curr_pfd;
	return GENERIC_FAIL;
}
