global _start
%include "macro.inc"

%define pc r15		;pointer to a next command
%define w r14		;pointer to a current command
%define rstack r13	;pointer to a return stack

section .text

%include "base.inc"
%include "outer_interpreter.inc"

section .bss

resq 1023				;end of the return stack
rstack_start: resq 1	;start of the return stack

user_mem: resq 65536	;global data for user

section .data

last_word: dq _lw

dp: dq user_mem		;current addr of global data
stack_start: dq 0	;stores a saved address of data stack

section .text

_start:
	mov rstack, rstack_start
	mov [stack_start], rsp

	mov pc, forth_init
	
; inner interpreter loop
next:
	mov w, pc
	add pc, 8
	mov w, [w]
	jmp [w]