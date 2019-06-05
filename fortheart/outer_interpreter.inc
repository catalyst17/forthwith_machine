section .bss
word_buf:  resq 512
user_dict:  resq 65536

section .data
here: dq user_dict
fd: dq 0

%define STATE_COMPILE 1
%define STATE_INTERPRET 0
state: dq STATE_INTERPRET

const "forth-here", here
const "forth-last-word", last_word
const "forth-is-compiling", state
const "forth-input-fd", fd
const "forth-stack-start", stack_start

; read word to buffer
; ( fd buffer -- )
colon "file-read-word", file_read_word
  dq xt_dup, xt_push_to_r, xt_push_to_r

; increments TOS
colon "inc", inc
  dq xt_lit, 1, xt_plus, xt_exit
  
; double dup for convinience (duplicates two top values)	  
colon "2dup", ddup
  dq xt_push_to_r, xt_dup, xt_fetch_r, xt_swap, xt_pop_from_r, xt_exit
  
; double increment (increments two top values)
colon "2inc", dinc
  dq xt_push_to_r, xt_inc, xt_pop_from_r, xt_inc, xt_exit

; counts length of the string
colon "string-length", string_length
      dq xt_dup
.loop:
      dq xt_dup, xt_fetch_char
      branch0 .exit
      dq xt_inc
      branch .loop
.exit:
  dq xt_swap, xt_sub, xt_exit
  
; compares strings
colon "string-equals", string_equals
.start:
  dq xt_ddup, xt_fetch_char, xt_swap, xt_fetch_char, xt_equal
  branch0 .no
  dq xt_dup, xt_fetch_char
  branch0 .yes
  dq xt_dinc
  branch .start
.yes:
  dq xt_drop, xt_drop, xt_lit, 1, xt_exit
.no:
  dq xt_drop, xt_drop, xt_lit, 0, xt_exit

  
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  

; adds word to dictionary (increments the word length)
colon "forth-dict-word-name", forth_dict_word_name
  dq xt_lit, 9, xt_plus, xt_exit

; checks if there input word in dictinory
colon "forth-find-word", forth_find_word
  dq xt_push_to_r, xt_last_word, xt_fetch
  
  dq xt_state, xt_fetch
  branch0 .loop
  
  dq xt_fetch
  
.loop:
  dq xt_dup
  branch0 .not_found

  dq xt_dup, xt_forth_dict_word_name, xt_fetch_r, xt_string_equals, xt_not
  branch0 .found

  dq xt_fetch
  branch .loop
  
.found:
  dq xt_lit, 1
.not_found:
  dq xt_pop_from_r, xt_drop, xt_exit

; converts word header start address to the execution token
; ( word_addr -- xt )
colon "forth-cfa", forth_cfa
  dq xt_lit, 9, xt_plus
  dq xt_dup, xt_string_length, xt_plus
  dq xt_lit, 2, xt_plus, xt_exit

; checks if interpreted word is immediate
colon "forth-xt-is-immediate", forth_xt_is_immediate
    dq xt_lit, 1, xt_sub, xt_fetch_char, xt_exit
  
; ( str -- )
; accepts word and tries to interpret it as a command, special command or a number
colon "forth-interpret", forth_interpret
.start:
    ; ( inbuff )
    dq xt_dup, xt_forth_find_word         ; try find input word in dictionary
    branch0 .number                       ; if not found then maybe it's number?
    dq xt_swap, xt_drop, xt_forth_cfa     ; if found - convert word address to XT of the word

    dq xt_state, xt_fetch
    branch0 .interpreting				  ; interpreting state?

    dq xt_dup, xt_forth_xt_is_immediate, xt_not
    branch0 .interpreting				  ; if immediate word - interpret it

.compiling:
    dq xt_comma, xt_exit
.interpreting:
    dq xt_execute, xt_exit

.number: ; ( inbuf )
    dq xt_dup, xt_string_parse_number      ; ( inbuf num? 0/len )
    branch0 .no_word

    ; ( inbuf num )
    dq xt_swap, xt_drop
    dq xt_state, xt_fetch                  ; if interpreting, that's it
    branch0 .exit

    dq xt_forth_follows_branch, xt_not
    branch0 .is_branch

.not_branch:
    dq xt_lit, xt_lit, xt_comma
.is_branch: ; ( num prev_xt )
    dq xt_comma, xt_exit

.no_word: ; ( inbuf )
  dq xt_lit, msg.no_such_word, xt_print_string, xt_print_string, xt_print_cr

.exit:
    dq xt_exit

	
	
	
	
	
	

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