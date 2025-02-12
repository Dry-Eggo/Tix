
section .bss

	std_print_string.m resb 1024
	std_print_int.n resb 1024
	glob_print_str.msg resb 1024
section .data

section .text

extern std_terminate_process
extern std_print_string
extern std_flush
extern std_print_int
extern std_copy
extern std_clear_string

	extern std_print_string
	extern std_flush
	extern std_print_int
glob_print_str:

	push rbp
	mov rbp, rsp
	mov [rbp -8], rdi

	mov rdi, glob_print.msg
	 call std_print_string
	 call std_flush
	pop rbp

	ret