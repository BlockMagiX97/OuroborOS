#ifndef OUROBOROS_ARCH_X86_64_KPANIC_H
#define OUROBOROS_ARCH_X86_64_KPANIC_H
#include <arch/kpanic.h>
#include <interrupts/isr.h>
__attribute__((noreturn)) void kpanic_interrupt(const struct interrupt_frame *in_fr, const char *msg);
#endif
