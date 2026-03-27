#include <arch/x86_64/hhdm.h>
#include <arch/x86_64/gdt.h>
#include <arch/x86_64/interrupts/idt.h>

// we want to share some stuff amongst cpus that only need to be loaded afterwards
void init_boot_cpu() {
	init_hhdm();
	init_gdt();
	idt_init();
}
void init_other_cpu() {
	init_shared_gdt();
	idt_shared_init();
}
void init_memory_manager();
void init_heap();
