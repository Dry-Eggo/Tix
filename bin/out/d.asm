
section .bss

	std_print_int.i resb 1024
	std_terminate_process.i resb 1024
	get.ret_val resd 1
	main.i resd 1
	main.limit resd 1
section .data

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

	extern std_print_int
	extern std_terminate_process
	extern std_flush
get:

	push rbp

	mov rbp, rsp

	mov dword [get.ret_val], 10
	pop rbp

	mov rax, 10
	ret
main:

	push rbp

	mov rbp, rsp

	mov dword [main.i], 1
	mov ax, [main.i]
	mov cx, 10000
	cmp ax, cx
	jl while0
	jge while0end
while0:
	mov rdi, [main.i]
	call std_print_int

	call std_flush

	mov dword [main.limit], 4
	call get

	push rax
	pop rax
	push rax
	push rcx
	mov rcx, [main.i]
	add rcx, rax
	mov [main.i], rcx
	pop rcx
	pop rax
	mov ax, [main.i]
	mov cx, 10000
	cmp ax, cx
	jl while0
	jle while0end
while0end:
	pop rbp

	call std_terminate_process

section .note.GNU-stack