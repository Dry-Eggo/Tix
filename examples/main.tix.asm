
section .bss

	std_print_string.m resb 1024
	std_print_int.m resb 1024
	std_getch.m resb 1024
	std_getch.dst resb 1024
	std_print_ch.m resb 1024
	for0.i resd 1
section .data

section .text
global _start

	extern std_terminate_process
_start:


	mov rbp, rsp
	call main

	extern std_print_string
	extern std_print_int
	extern std_flush
	extern std_getch
	extern std_print_ch
	mov dword [for0.i], 0
main:
	sub rsp, 1
	mov qword [rbp +8], rsp
	mov qword [rbp + 16], 0
	push rax
	push r9
	push r8

for0:
	mov cx, [for0.i]
	mov ax, 10
	cmp rcx, rax
	jge for0end
	mov rdi, qword [rbp + 8]
	mov rsi, [rbp + 8]
	call std_getch
	mov rdi, qword [rbp + 8]
	call std_print_ch
	mov eax, 1
	add dword [for0.i], eax
	jmp for0
nop
for0end:
	pop r9
	pop r9
	pop rax

	call std_terminate_process

section .note.GNU-stack