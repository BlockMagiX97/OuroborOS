#include <limine.h>
#include <stdint.h>

#include <mem.h>

// clang-format off
__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
	.id = LIMINE_HHDM_REQUEST_ID,
	.revision = 0
};
// clang-format on

uint64_t offset_hhdm = 0;
void init_hhdm() {
	offset_hhdm = hhdm_request.response->offset;
}

vaddr_t hhdm_phys_to_virt(paddr_t phys_addr) {
	return phys_addr + offset_hhdm;
}
paddr_t hhdm_virt_to_phys(vaddr_t virt_addr) {
	return virt_addr - offset_hhdm;
}
