; stack commands

; drops the top of the stack
; ( a -- )
native "drop", drop
	add rsp, 8
	jmp next