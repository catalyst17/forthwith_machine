; stack commands

; drops the top of the stack
; ( a -- )
native "drop", drop
	add rsp, 8
	jmp next

;swaps top and pre-top of the stack
; ( a b -- b a )
native "swap", swap
	pop rax
	xchg [rsp], rax
	push rax
	jmp next

; duplicates top of the stack
; ( a -- a a )
native "dup", dup
	push qword[rsp]
	jmp next
	

; ariphmetic

; sums two top values on the stack and put the result on top
; ( b a -- [a+b] )
native "+", plus
	pop rax
	add [rsp], rax
	jmp next

; ( b a -- [a*b] )
native "*", mul
	pop rax
	mul qword[rsp]
	pop rdx
	push rax
	jmp next

; ( b a -- [a div b] )
native "/", div
	pop rcx		; b
	pop rax		; a
	cqo 		; copies sign bit of rax into every bit of rdx
	div rcx		; a div b
	push rax
	jmp next

; ( b a -- [a mod b] )
native "%", mod
	pop rcx
	pop rax
	cqo
	div rcx
	push rdx
	jmp next

; ( b a -- [a - b] )
native "-", sub
	pop rax
	sub [rsp], rax
	jmp next