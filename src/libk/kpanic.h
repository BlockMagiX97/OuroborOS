#ifndef OUROBOROS_LIBK_KPANIC_H
#define OUROBOROS_LIBK_KPANIC_H
#include <isr.h>
__attribute__((noreturn)) void kpanic_interrupt(const struct interrupt_frame *in_fr, const char *msg);
#endif
