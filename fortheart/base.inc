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
; (a b -- [a+b] )
native "+", plus
	pop rax
	add [rsp], rax
	jmp next

; ( a b -- [a*b] )
native "*", mul
	pop rax
	mul qword[rsp]
	pop rdx
	push rax
	jmp next

; ( a b -- [a div b] )
native "/", div
	pop rcx		; b
	pop rax		; a
	cqo 		; copies sign bit of rax into every bit of rdx
	div rcx		; a div b
	push rax
	jmp next

; ( a b -- [a mod b] )
native "%", mod
	pop rcx
	pop rax
	cqo
	div rcx
	push rdx
	jmp next

; ( a b -- [a - b] )
native "-", sub
	pop rax
	sub [rsp], rax
	jmp next
	
; comparison
	
; ( a b -- c )
; if a == b then c = 1
; if a != b then c = 0
native "=", equal
	pop rax
	pop rdx
	cmp rax, rdx
	je .equals
	push 0
	jmp next
.equals:
	push 1
	jmp next
	
; ( a b -- c )
; if a < b then c = 1
; if a >= b then c = 0
native "<", less
	pop rdx
	pop rax
	cmp rax, rdx
	jl .less
	push 0
	jmp next
.less:
	push 1
	jmp next

; ( a b -- c )
; if a > b then c = 1
; if a <= b then c = 0
native ">", greater
	pop rdx
	pop rax
	cmp rax, rdx
	jg .greater
	push 0
	jmp next
.greater:
	push 1
	jmp next

	
; logic

; ( a -- a' )
; if a == 0 then a' = 1
; if a != 0 then a' = 0
native "not", not
	pop rax
    test rax, rax
    setz al			;sets the byte to 1 if zero-flag, otherwise to 0
    movzx rax, al
    push rax	
	jmp next

; ( b a -- a||b )
native "lor", lor
	pop rax
	pop rdx
	test rax, rax
	jnz .true
	test rdx, rdx
	jnz .true
	push 0
	jmp next
.true:
	push 1
	jmp next

; ( b a -- a&&b )
native "land", land
	pop rax
	pop rdx
	test rax, rax
	jz .false
	test rdx, rdx
	jz .false
	push 1
	jmp next
.false:
	push 0
	jmp next
	
	
; bitwise

; ( a b -- a&b )
native "and", and
	pop rax
	pop rdx
	and rdx, rax
	push rax
	jmp next

; ( a b -- a|b )
native "or", or
	pop rax
	pop rdx
	or rdx, rax
	push rax
	jmp next
	
	
; return stack commands

; drop the top of the dstack and place it to the rstack
native ">r", push_to_r
	pop rax
	rpush rax
	jmp next

; drop the top of the rstack and place it to the dstack
native "r>", pop_from_r
	rpop rax
	push rax
	jmp next

; copy the top of the rstack and place it to the dstack
native "r@", fetch_r
	push qword[rstack]
	jmp next
	

; memory

; ( addr -- value )
native "@", fetch
	pop rax
	push qword[rax]
	jmp next

; ( val addr -- )
native "!", store
	pop rax			; addr
	pop rdx			; val
	mov [rax], rdx
	jmp next

; ( char addr -- )
native "c!", store_char
	pop rdx			; addr
	pop rax			; char
	mov [rdx], al
	jmp next

; ( addr -- char )
native "c@", fetch_char
	pop rax
	movzx rax,  byte[rax]
	push rax
	jmp next

; execute word word, execution token of which is on TOS
; ( xt -- )	
native "execute", execute
	pop rax
	mov w, rax
	jmp [rax]

native "forth-dp", forth_dp
	push dp
	jmp next
	

; maintaining the execution

; command to prepare for the colon word execution
native "docol", docol
	rpush pc
	add w, 8
	mov pc, w
	jmp next

; jump to a location (absolute address)
native "branch", branch
	mov pc, [pc]
	jmp next

; jump to a location (absolute address) if TOS == 0
native "0branch", branch0
	pop rax
	test rax,rax
	jnz .skip
	mov pc, [pc]
	jmp next
.skip:
	add pc, 8
	jmp next

; command to exit the colon word defining
native "exit", exit
	rpop pc
	jmp next
	
	
; service

; pushes a value immediately following this XT
native "lit", lit
	push qword[pc]
	add pc, 8
	jmp next

; places pointer to the stack on TOS
native "forth-sp", forth_sp
	push rsp
	jmp next

; places pointer to the stack_base on TOS
native "forth-stack-base", forth_stack_base
	push qword[stack_start]
	jmp next
	

; executes syscall
; the following registers store arguments (relatively to its order)
; rdi , rsi , rdx , r10 , r8 and r9
; ( call_num a1 a2 a3 a4 a5 a6 -- new_rax new_rdx )
native "syscall", syscall
	pop r9
	pop r8
	pop r10
	pop rdx
	pop rsi
	pop rdi
	pop rax
	syscall
	push rax
	push rdx
	jmp next

; restore forth's stack to its initial state	
native "forth-stack-restore", forth_stack_restore
  mov rsp, [stack_start]
  jmp next