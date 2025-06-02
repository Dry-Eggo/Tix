
extern t_main
extern texit
global _start
_start:
        call t_main
        mov rdi, 0
        call texit
        ret

global __tsys0
__tsys0:
        syscall
        ret


        
global __tsys1
__tsys1:
        mov rax, rdi
        syscall
        ret

        
global __tsys2
__tsys2:
        mov rax, rdi
        mov rdi, rsi
        syscall
        ret
global __tsys3
__tsys3:
        mov rax, rdi
        mov rdi, rsi
        mov rsi, rdx
        syscall
        ret
global __tsys4
__tsys4:
        mov rax, rdi
        mov rdi, rsi
        mov rsi, rdx
        mov rdx, rcx
        syscall
        ret
global __tsys5
__tsys5:
        mov rax, rdi
        mov rdi, rsi
        mov rsi, rdx
        mov rdx, rcx
        mov r10, r8
        syscall
        ret
global __tsys6
__tsys6:
        mov rax, rdi
        mov rdi, rsi
        mov rsi, rdx
        mov rdx, rcx
        mov r10, r8
        mov r8, r9
        mov r9, [rsp + 8]
        syscall
        ret
section .note.GNU-stack
