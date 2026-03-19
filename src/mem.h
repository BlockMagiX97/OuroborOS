#ifndef OUROBOROS_MEM_H
#define OUROBOROS_MEM_H

#include <stdint.h>

typedef uint64_t paddr_t;
typedef uint64_t vaddr_t;
extern uint64_t offset_hhdm;

void init_hhdm();
vaddr_t hhdm_phys_to_virt(paddr_t phys_addr);
paddr_t hhdm_virt_to_phys(vaddr_t virt_addr);

#endif
