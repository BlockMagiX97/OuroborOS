#ifndef OUROBOROS_ARCH_KPANIC_H
#define OUROBOROS_ARCH_KPANIC_H
__attribute__((noreturn)) void kpanic(const char *heading, const char *msg);
#endif
