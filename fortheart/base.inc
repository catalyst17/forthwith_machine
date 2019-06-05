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