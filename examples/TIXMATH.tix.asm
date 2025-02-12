
section .bss

	glob_add_int_int.a resb 1024
	glob_add_int_int.b resb 1024
section .data

section .text

extern std_terminate_process
extern std_print_string
extern std_flush
extern std_print_int
extern std_copy
extern std_clear_string

glob_add_int_int:

	push rbp
	mov rbp, rsp
	mov [rbp -8], rdi

	mov [rbp -16], rsi

	pop rbp

	mov rax, rax
	ret
	ret