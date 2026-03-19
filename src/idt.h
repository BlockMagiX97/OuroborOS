#ifndef OUROBOROS_IDT_H
#define OUROBOROS_IDT_H
#include <stdint.h>
#include <stddef.h>
#include <gdt.h>
#define NUM_IDT_ENTRIES 256

struct interrupt_descriptor_entry {
	uint16_t offset_low;	// offset bits 0..15
	uint16_t selector;	// a code segment selector in GDT or LDT
	uint8_t ist;		// bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
	uint8_t type_attributes;// gate type, dpl, and p fields
	uint16_t offset_mid;	// offset bits 16..31
	uint32_t offset_high;	// offset bits 32..63
	uint32_t zero;		// reserved
} __attribute__((packed));
extern struct interrupt_descriptor_entry idt_table[NUM_IDT_ENTRIES];

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void idt_set_gate(size_t index, uint64_t offset, segment_selector_t selector, uint8_t ist, uint8_t type_attributes);
void init_idt();
#endif
