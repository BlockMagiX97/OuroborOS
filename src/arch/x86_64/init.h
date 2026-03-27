#ifndef OUROBOROS_ARCH_X86_64_INIT_H
#define OUROBOROS_ARCH_X86_64_INIT_H

void init_boot_cpu();
void init_other_cpu();
void init_memory_manager();
void init_heap();

#endif
