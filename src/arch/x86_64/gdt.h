#ifndef OUROBOROS_ARCH_X86_64_GDT_H
#define OUROBOROS_ARCH_X86_64_GDT_H
#include <stdint.h>
#include <stddef.h>

#define MAX_NUM_GDT_ENTRIES 3
typedef uint16_t segment_selector_t;

extern segment_selector_t kernel_code_segment_sel;
extern segment_selector_t kernel_data_segment_sel;

void init_gdt();
void init_shared_gdt();
#endif
