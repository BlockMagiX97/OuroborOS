#ifndef OUROBOROS_ARCH_X86_64_KPANIC_H
#define OUROBOROS_ARCH_X86_64_KPANIC_H
#include <arch/x86_64/interrupts/isr.h>
__attribute__((noreturn)) void kpanic_interrupt(const struct interrupt_frame *in_fr, const char *msg);
__attribute__((noreturn)) void kpanic(const char *msg);
#endif
