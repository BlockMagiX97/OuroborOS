%macro pushall 0
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro popall 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
%endmacro
section .text
bits 64

extern isr_dispatch
isr_stub:
	pushall
	mov rdi, rsp ; first arg to interrupt_dispatch is struct regs *
	call isr_dispatch
	mov rsp, rax ; return struct regs *
	popall
	add rsp, 16 ; error_code + vector_num
	sti
	iretq


%macro isr_err_code 1
	push %1 
%endmacro

%macro isr_no_err_code 1
	push 0 ; dummy error code
	push %1 
%endmacro

%assign i 0
%rep 256
align 16
isr_ %+ i:
	cli ; disable interrupts
%if i == 8 || (i >= 10 && i <= 14) || i == 17 || i == 21 || i == 29 || i == 30
	isr_err_code i
%else
	isr_no_err_code i
%endif
	jmp isr_stub
%assign i i+1
%endrep

; expose them to C
%assign i 0
%rep 256
	global isr_ %+ i
%assign i i+1
%endrep
