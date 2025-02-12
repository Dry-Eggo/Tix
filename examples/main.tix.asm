
section .bss

	main.ad1 resd 1
	main.ad2 resd 1
	main.num resb 1024
section .data

	main.hello db "Course Repee" ,0
section .text
global _start

extern std_terminate_process
extern std_print_string
extern std_flush
extern std_print_int
extern std_copy
extern std_clear_string

_start:


	mov rbp, rsp
	call main

	%include "/home/dry/Documents/Eggo/Tix/bin/../examples/TIXIO.tix.asm"
	%include "/home/dry/Documents/Eggo/Tix/bin/../examples/TIXMATH.tix.asm"
main:
	push rbp
	mov rbp, rsp
	mov dword [main.ad1], 100
	mov dword [main.ad2], 2
	push rax
	mov rax, [main.ad1]
	mov [glob_add_int_int.a], rax

pop rax
	mov rdi, main.ad1
	push rax
	mov rax, [main.ad2]
	mov [glob_add_int_int.b], rax

pop rax
	mov rsi, main.ad2
	call glob_add_int_int

	push rax
	mov rax, [main.ad1]
	mov [glob_add_int_int.a], rax

pop rax
	mov rdi, main.ad1
	push rax
	mov rax, [main.ad2]
	mov [glob_add_int_int.b], rax

pop rax
	mov rsi, main.ad2
	call glob_add_int_int

	pop rax
	mov [main.num], rax
	push rax
	mov rax, [main.hello]
	mov [glob_print_str.msg], rax

pop rax
	mov rdi, main.hello
	call glob_print_str

	pop rbp

	call std_terminate_process

section .note.GNU-stack