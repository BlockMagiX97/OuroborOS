#ifndef OUROBOROS_ARCH_X86_64_GDT_H
#define OUROBOROS_ARCH_X86_64_GDT_H
#include <stdint.h>
#include <stddef.h>
#define MAX_NUM_GDT_ENTRIES 3

struct global_descriptor_table_entry {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t access;
	uint8_t flags;
	uint8_t base_high;
} __attribute__((packed));

typedef uint16_t segment_selector_t;

extern segment_selector_t kernel_code_segment_sel;
extern segment_selector_t kernel_data_segment_sel;
extern struct global_descriptor_table_entry gdt[MAX_NUM_GDT_ENTRIES];

int gdt_set_gate(size_t index, uint64_t base, uint32_t limit, uint8_t access, uint8_t flags);
void init_gdt();
void init_shared_gdt();
#endif
