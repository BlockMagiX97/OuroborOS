#include <stdint.h>
#include <libk/assert.h>
#include <gdt.h>
#include <idt.h>

struct interrupt_descriptor_entry idt_table[NUM_IDT_ENTRIES] = {0};
void idt_set_gate(size_t index, uint64_t offset, segment_selector_t selector, uint8_t ist, uint8_t type_attributes) {
	safety_assert(index < NUM_IDT_ENTRIES);
	idt_table[index].type_attributes = type_attributes;
	idt_table[index].ist = ist;
	idt_table[index].selector = selector;
	idt_table[index].offset_low = offset & 0xffff;
	idt_table[index].offset_mid = (offset << 16) & 0xffff;
	idt_table[index].offset_high = (offset << 32) & 0xffffffff;
}
void idt_init() {
}
