section .bss
word_buf:  resq 512
user_dict:  resq 65536

section .data
fd: dq 0

const "forth-input-fd", fd

; read word to buffer
; ( fd buffer -- )
colon "file-read-word", file_read_word
  dq xt_dup, xt_push_to_r, xt_push_to_r

; command to read  words from an open fd and interprets them using "forth-interpret"
; ( fd -- )
colon "forth-interpret-fd", forth_interpret_fd
    dq xt_lit, xt_fd, xt_fetch, xt_push_to_r
    dq xt_fd, xt_store

.loop:
    dq xt_fd, xt_fetch, xt_lit, word_buf, xt_file_read_word
    branch0 .eof

    dq xt_lit, word_buf, xt_forth_interpret

    branch .loop

.eof:
    dq xt_pop_from_r, xt_fd, xt_store, xt_exit


; initialize forth machine
forth_init:
	dq xt_lit, 0, xt_forth_interpret_fd, xt_bye