#include <limine.h>
#include <mem.h>
#include <stdbool.h>
#include <stdint.h>

#include <libk/string.h>
#include <libk/util.h>
#include <libk/wrap_builtin.h>

// clang-format off
__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST_ID,
	.revision = 0
};
// clang-format on

#define PAGE_SIZE 4096

// Here I will try to implement simple bitmap phys allocator
//
struct __attribute__((packed)) phys_mm_bitmap {
	paddr_t base;
	uint64_t num_pages;
	uint64_t num_free_pages;
	uint64_t first_free_index;

	// long num_pages BITS
	// 1 means used
	uint8_t bitmap[];
};
struct __attribute__((packed)) phys_mmap {
	uint64_t num_bitmaps;
	// phys_addrs of bitmaps ( they are immidietely after )
	paddr_t maps[];
};

paddr_t phys_mmap_addr;

void phys_init_allocator() {
	// first figure out how much memory is needed for the bitmap
	uint64_t size = sizeof(struct phys_mmap);

	struct limine_memmap_entry *mm_entry;
	uint64_t num_usable_regions = 0;
	for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
		mm_entry = *(memmap_request.response->entries) + i;
		if (mm_entry->type == LIMINE_MEMMAP_USABLE) {
			num_usable_regions++;
			size += sizeof(struct phys_mm_bitmap);
			// bitmap size
			size += mm_entry->length / PAGE_SIZE / 8 + 1;
		}
	}
	size += sizeof(paddr_t) * num_usable_regions;

	for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
		mm_entry = *(memmap_request.response->entries) + i;
		if (mm_entry->type == LIMINE_MEMMAP_USABLE) {
			if (mm_entry->length >= size) {
				goto found_space;
			}
		}
	}

	hcf();

found_space:
	// it is impossible for mm_entry to not be set at this stage
	phys_mmap_addr = mm_entry->base;
	void *ptr = (void *)hhdm_phys_to_virt(phys_mmap_addr);
	((struct phys_mmap *)ptr)->num_bitmaps = num_usable_regions;

	uint64_t j = 0;
	paddr_t phys_addr = phys_mmap_addr + sizeof(struct phys_mmap) + num_usable_regions * sizeof(paddr_t);

	for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
		mm_entry = *(memmap_request.response->entries) + i;
		if (mm_entry->type == LIMINE_MEMMAP_USABLE) {
			((struct phys_mmap *)ptr)->maps[j] = phys_addr;
			struct phys_mm_bitmap *curr_bitmap = (struct phys_mm_bitmap *)hhdm_phys_to_virt(phys_addr);
			curr_bitmap->base = mm_entry->base;
			curr_bitmap->num_pages = mm_entry->length / PAGE_SIZE;
			curr_bitmap->num_free_pages = curr_bitmap->num_pages;
			curr_bitmap->first_free_index = 0;
			// +1 since we round down ( this wastes a byte if num of pages is divisible by 8
			const uint64_t bitmap_byte_size = curr_bitmap->num_pages / 8 + 1;

			memset(&(curr_bitmap->bitmap), 0, bitmap_byte_size);
			if (phys_mmap_addr ==
			    mm_entry->base) { // we need to mark the location we use for metadata as used
				uint64_t num_pages_occupied = size / PAGE_SIZE;
				curr_bitmap->first_free_index = num_pages_occupied;
				curr_bitmap->num_free_pages = curr_bitmap->num_free_pages - num_pages_occupied;
				memset(&(curr_bitmap->bitmap), 0xff, num_pages_occupied / 8);
				curr_bitmap->bitmap[num_pages_occupied / 8] =
				    (uint8_t)(0xff << (8 - (num_pages_occupied % 8)));
			}

			phys_addr += sizeof(struct phys_mm_bitmap) + bitmap_byte_size;
			j++;
		}
	}
};

#define BITMAP_INDEX_SET(bitmap, index) \
	((bitmap)[(index) / sizeof(*(bitmap))] |= 1 << ((index) % sizeof(*(bitmap))))

#define BITMAP_INDEX_UNSET(bitmap, index) \
	((bitmap)[(index) / sizeof(*(bitmap))] &= ~(1 << ((index) % sizeof(*(bitmap)))))

#define BITMAP_INDEX_IS_USED(bitmap, index) \
	(!!((bitmap)[(index) / sizeof(*(bitmap))] & 1 << ((index) % sizeof(*(bitmap)))))

// alocates exactly num_pages continous pages in physical memory and returns address of the first page
paddr_t phys_alloc(uint64_t num_pages) {
	if (UNLIKELY(num_pages == 0)) {
		hcf();
	}
	struct phys_mmap *phys_mmap_ptr = (struct phys_mmap *)hhdm_phys_to_virt(phys_mmap_addr);
	for (uint64_t i = 0; i < phys_mmap_ptr->num_bitmaps; i++) {
		struct phys_mm_bitmap *curr_bitmap = (struct phys_mm_bitmap *)hhdm_phys_to_virt(phys_mmap_ptr->maps[i]);
		if (curr_bitmap->num_free_pages < num_pages) {
			continue;
		}
		uint64_t count_continuous_pages = 0;
		uint64_t spec_free_pages = curr_bitmap->num_free_pages;
		for (uint64_t j = curr_bitmap->first_free_index; j < curr_bitmap->num_pages; j++) {
			if (!BITMAP_INDEX_IS_USED(curr_bitmap->bitmap, j)) {
				count_continuous_pages++;
				if (count_continuous_pages >= num_pages) {

					// mark pages as used
					for (uint64_t k = j - num_pages; k < j; k++) {
						BITMAP_INDEX_SET(curr_bitmap->bitmap, k);
					}
					// update metadata
					curr_bitmap->num_free_pages -= num_pages;
					if (j - num_pages == curr_bitmap->first_free_index) {
						curr_bitmap->first_free_index = j;
						for (; curr_bitmap->first_free_index < curr_bitmap->num_pages &&
						       BITMAP_INDEX_IS_USED(curr_bitmap->bitmap, j);
						     curr_bitmap->first_free_index++)
							;
					}

					return curr_bitmap->base + (j - num_pages) * PAGE_SIZE;
				}
			} else {
				if (spec_free_pages < count_continuous_pages) {
					UNREACHABLE;
				}
				spec_free_pages -= count_continuous_pages;
				if (spec_free_pages < num_pages) {
					break;
				}
				count_continuous_pages = 0;
			}
		}
	}
	return 0;
}
void phys_free(paddr_t addr, uint64_t num_pages) {
	if (addr == 0) {
		return;
	}
	if (UNLIKELY((addr & (PAGE_SIZE - 1)) != 0)) {
		hcf();
	}
	if (UNLIKELY(num_pages == 0)) {
		hcf();
	}
	uint64_t page_index;
	struct phys_mm_bitmap *curr_bitmap;
	struct phys_mmap *phys_mmap_ptr = (struct phys_mmap *)hhdm_phys_to_virt(phys_mmap_addr);
	for (uint64_t i = 0; i < phys_mmap_ptr->num_bitmaps; i++) {
		curr_bitmap = (struct phys_mm_bitmap *)hhdm_phys_to_virt(phys_mmap_ptr->maps[i]);
		if (addr > curr_bitmap->base && addr < (curr_bitmap->base + curr_bitmap->num_pages * PAGE_SIZE)) {
			goto found_associated_bitmap;
		}
	}
	// possible to reach if user provides incorrect address
	hcf();
found_associated_bitmap:
	page_index = addr - curr_bitmap->base;
	for (uint64_t i = 0; i < num_pages; i++) {
		if (UNLIKELY(page_index + i >= curr_bitmap->num_pages)) {
			hcf();
		}
		if (UNLIKELY(!BITMAP_INDEX_IS_USED(curr_bitmap->bitmap, page_index + i))) {
			hcf();
		}
		BITMAP_INDEX_UNSET(curr_bitmap->bitmap, page_index + i);
	}
}
