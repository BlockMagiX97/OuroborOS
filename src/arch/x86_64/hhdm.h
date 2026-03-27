#ifndef OUROBOROS_ARCH_X86_64_HHDM_H
#define OUROBOROS_ARCH_X86_64_HHDM_H

#include <stdint.h>
#include <arch/x86_64/mem_addr.h>

extern uint64_t offset_hhdm;

void init_hhdm();
vaddr_t hhdm_phys_to_virt(paddr_t phys_addr);
paddr_t hhdm_virt_to_phys(vaddr_t virt_addr);

#endif
