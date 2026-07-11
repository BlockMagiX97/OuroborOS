#include <libk/stdio.h>
#include <hcf.h>
#include <interrupts/isr.h>
#include <kpanic.h>
#include <output/debug.h>

#define RED_FG "\033[38;2;255;0;0m"
#define RED_BG "\033[48;2;255;0;0m"

#define WHITE_FG "\033[38;2;255;255;255m"
#define WHITE_BG "\033[48;2;255;255;255m"

#define BLACK_FG "\033[38;2;0;0;0m"
#define BLACK_BG "\033[48;2;0;0;0m"
#define RESET "\033[0m"

__attribute__((noreturn)) void kpanic_interrupt(const struct interrupt_frame *in_fr, const char *msg) {
	flags_t flags;
	lock_debug_output(&flags);
	printf_limited_custom_func(send_debug_output_no_lock,RED_BG WHITE_FG "KERNEL PANIC:" BLACK_BG RED_FG " %s" RESET "\n", msg);
	printf_limited_custom_func(send_debug_output_no_lock,"\n");
	printf_limited_custom_func(send_debug_output_no_lock,"RAX= 0x%016lx  RBX= 0x%016lx  RCX= 0x%016lx  RDX= 0x%016lx\n", in_fr->rax, in_fr->rbx, in_fr->rcx, in_fr->rdx);
	printf_limited_custom_func(send_debug_output_no_lock,"RDI= 0x%016lx  RSI= 0x%016lx  RBP= 0x%016lx  RSP= 0x%016lx\n", in_fr->rax, in_fr->rbx, in_fr->rbp, in_fr->userrsp);
	printf_limited_custom_func(send_debug_output_no_lock,"R8 = 0x%016lx  R9 = 0x%016lx  R10= 0x%016lx  R11= 0x%016lx\n", in_fr->r8, in_fr->r9, in_fr->r10, in_fr->r11);
	printf_limited_custom_func(send_debug_output_no_lock,"R12= 0x%016lx  R13= 0x%016lx  R14= 0x%016lx  R15= 0x%016lx\n", in_fr->r12, in_fr->r13, in_fr->r14, in_fr->r15);
	printf_limited_custom_func(send_debug_output_no_lock,"\n");
	printf_limited_custom_func(send_debug_output_no_lock,"CS= 0x%016lx  SS= 0x%016lx\n", in_fr->cs, in_fr->ss);
	printf_limited_custom_func(send_debug_output_no_lock,"RFLAGS= 0x%016lx  RIP= 0x%016lx\n", in_fr->rflags, in_fr->rip);
	printf_limited_custom_func(send_debug_output_no_lock,"INT_NO= %ld  ERR= %ld\n", in_fr->int_no, in_fr->err_code);
	unlock_debug_output(&flags);
	hcf();
}
__attribute__((noreturn)) void kpanic(const char *heading, const char *msg) {
	printf_limited(RED_BG WHITE_FG "KERNEL PANIC:" BLACK_BG RED_FG " %s" RESET "\n%s", heading, msg);
	hcf();
}
