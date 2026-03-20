#ifndef OUROBOROS_ISR_H
#define OUROBOROS_ISR_H
#include <stdint.h>

// A struct to hold the register state passed from our assembly stubs.
// The order MUST match the order of the push instructions in interrupt.asm
struct interrupt_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rsi, rdi, rbp, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code; // Pushed by our stubs and the CPU.
    uint64_t rip, cs, rflags, userrsp, ss; // Pushed by the CPU automatically.
}__attribute__((packed));
void isr_install();
#endif
