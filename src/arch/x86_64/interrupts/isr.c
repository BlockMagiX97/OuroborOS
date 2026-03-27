#include <arch/x86_64/gdt.h>
#include <arch/x86_64/interrupts/idt.h>
#include <arch/x86_64/interrupts/isr.h>
#include <arch/x86_64/mem_addr.h>
#include <arch/x86_64/kpanic.h>

#include <libk/string.h>

extern void isr_0();
extern void isr_1();
extern void isr_2();
extern void isr_3();
extern void isr_4();
extern void isr_5();
extern void isr_6();
extern void isr_7();
extern void isr_8();
extern void isr_9();
extern void isr_10();
extern void isr_11();
extern void isr_12();
extern void isr_13();
extern void isr_14();
extern void isr_15();
extern void isr_16();
extern void isr_17();
extern void isr_18();
extern void isr_19();
extern void isr_20();
extern void isr_21();
extern void isr_22();
extern void isr_23();
extern void isr_24();
extern void isr_25();
extern void isr_26();
extern void isr_27();
extern void isr_28();
extern void isr_29();
extern void isr_30();
extern void isr_31();

const char *exception_name_table[32] = {
    "Divide error",
    "Debug exception",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Bound range exceded",
    "Invalid opcode",
    "Device not avalible",
    "Double fault",
    "Coprocessor Segment Overrun (reserved)",
    "Invalid TSS",
    "Segment not present",
    "Stack Segment fault",
    "General Protection",
    "Page Fault",
    "Intel reserved",
    "x87 FPU Floating-Point Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "UNDEFINED",
    "UNDEFINED",
    "UNDEFINED",
    "UNDEFINED",
    "UNDEFINED",
    "UNDEFINED",
    "UNDEFINED",
    "UNDEFINED",
    "UNDEFINED",
    "UNDEFINED",
};
struct interrupt_frame *handle_exceptions(struct interrupt_frame *in_fr) {
	switch (in_fr->int_no) {
		default:
			kpanic_interrupt(in_fr, exception_name_table[in_fr->int_no]);
	}
	return in_fr;
}
struct interrupt_frame *isr_dispatch(struct interrupt_frame *in_fr) {
	if (in_fr->int_no < 32) {
		return handle_exceptions(in_fr);
	}
	return in_fr;
}
#define IDT_TA_GATE_TYPE_INTERRUPT (0b1110)
#define IDT_TA_GATE_TYPE_TRAP (0b1111)
#define IDT_TA_DLP(ring) (((ring)&0b11) << 5)
#define IDT_TA_PRESENT (1 << 7)

#define IDT_TA_GATE_TYPE_ABORT (IDT_TA_GATE_TYPE_TRAP)
#define IDT_TA_GATE_TYPE_FAULT (IDT_TA_GATE_TYPE_TRAP)

void isr_install_exceptions() {

#define SET_GATE_RING_0(index, type) \
	do {idt_set_gate((index), (vaddr_t)isr_##index, kernel_code_segment_sel, 0, IDT_TA_GATE_TYPE_##type | IDT_TA_DLP(0) | IDT_TA_PRESENT);} while(0)

	SET_GATE_RING_0(0, FAULT);     // Divide error
	SET_GATE_RING_0(1, TRAP);      // Debug exception
	SET_GATE_RING_0(2, INTERRUPT); // NMI
	SET_GATE_RING_0(3, TRAP);      // Breakpoint
	SET_GATE_RING_0(4, TRAP);      // Overflow
	SET_GATE_RING_0(5, FAULT);     // Bound range exceded
	SET_GATE_RING_0(6, FAULT);     // Invalid opcode
	SET_GATE_RING_0(7, FAULT);     // Device not avalible
	SET_GATE_RING_0(8, ABORT);     // Double fault
	SET_GATE_RING_0(9, FAULT);     // Coprocessor Segment Overrun (reserved)
	SET_GATE_RING_0(10, FAULT);    // Invalid TSS
	SET_GATE_RING_0(11, FAULT);    // Segment not present
	SET_GATE_RING_0(12, FAULT);    // Stack Segment fault
	SET_GATE_RING_0(13, FAULT);    // General Protection
	SET_GATE_RING_0(14, FAULT);    // Page Fault
				       // Intel reserved
	SET_GATE_RING_0(16, FAULT);    // x87 FPU Floating-Point Error
	SET_GATE_RING_0(17, FAULT);    // Alignment Check
	SET_GATE_RING_0(18, ABORT);    // Machine Check
	SET_GATE_RING_0(19, FAULT);    // SIMD Floating-Point Exception
	SET_GATE_RING_0(20, FAULT);    // Virtualization Exception
	SET_GATE_RING_0(21, FAULT);    // Control Protection Exception

#undef SET_GATE_RING_0
}
void isr_install() {
	isr_install_exceptions();
}
