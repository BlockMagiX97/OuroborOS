#include <arch/x86_64/hcf.h>
// Halt and catch fire function.
__attribute__((noreturn)) void hcf(void) {
	for (;;) {
		asm("hlt");
	}
}
