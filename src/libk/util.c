// Halt and catch fire function.
__attribute__((noreturn)) void hcf(void) {
	for (;;) {
		asm("hlt");
	}
}
