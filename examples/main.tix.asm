
section .bss

	tixc_print.m resb 1024
	tixc_print_buf.m resb 1024
	tixc_print_int.i resb 1024
	tixc_read.m resb 1024
	tixc_strjoin.m resb 1024
	tixc_strjoin.d resb 1024
	tixc_strjoin.e resb 1024
	strcat.m resb 1024
	strcat.d resb 1024
	printf.fmt resb 1024
	_Tix_main.tix_input.m resb 1024
	_Tix_main.tix_input.inp resb 1024
	_Tix_main.tix_main.out resb 1024
section .data

	gbval0 db "What is your name : ", 0
	gbval1 db "Hello, ", 0
section .text
global main

	extern std_terminate_process

	mov rbp, rsp
	call main

	extern tixc_print
	extern tixc_print_buf
	extern tixc_print_int
	extern tixc_read
	extern tixc_strjoin
	extern strcat
	extern printf
_Tix_main.tix_input_strE:

	push rbp
	mov rbp, rsp
	mov [rbp -8], rdi

	mov rdi, [rbp - 8]
	call tixc_print
	mov rdi, _Tix_main.tix_input.inp
	call tixc_read
	mov rax, _Tix_main.tix_input.inp
	pop rbp
	ret
	pop rbp 
	ret
main:
	push rbp
	mov rbp, rsp
	mov qword [rbp - 8], gbval0
	mov rdi, gbval0
	call _Tix_main.tix_input_strE
	mov qword [rbp - 16], rax
	mov qword [rbp - 32], gbval1
	mov rdi, _Tix_main.tix_main.out
	mov rsi, [rbp - 32]
	mov rdx, [rbp - 16]
	call tixc_strjoin
	mov rdi, _Tix_main.tix_main.out
	call tixc_print
	mov rdi, 20
	call std_terminate_process

section .note.GNU-stack