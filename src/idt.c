#include <stdint.h>

#include <libk/assert.h>

#include <gdt.h>
#include <idt.h>
#include <isr.h>

extern void idt_load(struct idt_ptr* idt_ptr);
struct interrupt_descriptor_entry idt_table[NUM_IDT_ENTRIES] = {0};
void idt_set_gate(size_t index, uint64_t offset, segment_selector_t selector, uint8_t ist, uint8_t type_attributes) {
	safety_assert(index < NUM_IDT_ENTRIES);
	idt_table[index].type_attributes = type_attributes;
	idt_table[index].ist = ist;
	idt_table[index].selector = selector;
	idt_table[index].offset_low = offset & 0xffff;
	idt_table[index].offset_mid = (offset >> 16) & 0xffff;
	idt_table[index].offset_high = (offset >> 32) & 0xffffffff;
}
struct idt_ptr idt_ptr;
void idt_init() {
	isr_install();

	idt_ptr.base = (uint64_t)&idt_table;
	idt_ptr.limit = sizeof(idt_table);
	idt_load(&idt_ptr);
	asm ("sti"); // enable interrupts
}
