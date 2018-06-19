
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

_NR_get_ticks       equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！
_NR_process_sleep    equ  1
_NR_disp_str_s         equ 2 
_NR_sem_p            equ  3
_NR_sem_v            equ  4

INT_VECTOR_SYS_CALL equ 0x90

; 导出符号
global	get_ticks
global  process_sleep
global  disp_str_s
global  sem_p
global  sem_v


bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
        push  ebx
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
        pop   ebx
	ret

process_sleep:
        push  ebx
        mov   ebx,[esp+8]
        mov   eax,_NR_process_sleep
        int     INT_VECTOR_SYS_CALL
        pop  ebx
        ret

disp_str_s:
        push ebx
        mov   ebx,[esp+8]
        mov   eax,_NR_disp_str_s
        int    INT_VECTOR_SYS_CALL
        pop ebx
        ret

sem_p:
        push ebx
        mov  ebx,[esp+8]
        mov   eax,_NR_sem_p
        int   INT_VECTOR_SYS_CALL
        pop  ebx
        ret

sem_v:
        push ebx
        mov  ebx,[esp+8]
        mov   eax,_NR_sem_v
        int   INT_VECTOR_SYS_CALL
        pop  ebx
        ret
        


