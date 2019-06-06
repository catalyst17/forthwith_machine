section .rodata
msg:
  .no_such_word: db "! No such word: ", 0

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

; IO & parsing utils for strings, chars and numbers

%define sys_read_no 0
%define sys_write_no 1

; read word to buffer
; ( fd buffer -- )
colon "file-read-word", file_read_word
  dq xt_dup, xt_push_to_r, xt_push_to_r

; makes TOS negative
colon "neg", neg
  dq xt_lit, 0, xt_swap, xt_sub, xt_exit
  
; increments TOS
colon "inc", inc
  dq xt_lit, 1, xt_plus, xt_exit
  
; double dup for convinience (duplicates two top values)	  
colon "2dup", ddup
  dq xt_push_to_r, xt_dup, xt_fetch_r, xt_swap, xt_pop_from_r, xt_exit
  
; double increment (increments two top values)
colon "2inc", dinc
  dq xt_push_to_r, xt_inc, xt_pop_from_r, xt_inc, xt_exit

; drop TOS if it's a zero
colon "drop0", drop0
  dq xt_dup
  branch0 .is_zero
  dq xt_exit
.is_zero:
  dq xt_drop, xt_exit
  
; checks if char on TOS is digit
colon "char-is-digit", char_is_digit
  dq xt_push_to_r
  dq xt_fetch_r, xt_lit, '9'+1, xt_less
  dq xt_lit, '0'-1, xt_fetch_r, xt_less, xt_land
  dq xt_pop_from_r, xt_drop, xt_exit

; parses digit from char by simply substracting the code of zero  
colon "char-to-digit", char_to_digit
  dq xt_lit, '0', xt_sub, xt_exit

; checks if input is a u-number and, if yes, does parsing
colon "string-parse-unsigned-number", string_parse_unsigned_number
  dq xt_dup, xt_push_to_r
  dq xt_lit, 0, xt_push_to_r
.loop:
  dq xt_dup, xt_fetch_char, xt_dup, xt_char_is_digit
  branch0 .end
  dq xt_char_to_digit
  dq xt_lit, 10, xt_pop_from_r, xt_mul, xt_plus, xt_push_to_r
  dq xt_inc
  branch .loop
  .end:
  dq xt_drop, xt_pop_from_r, xt_swap, xt_pop_from_r, xt_sub, xt_drop0, xt_exit
  
; checks if input is a number and, if yes, does parsing
; ( buf -- num? len )
colon "string-parse-number", string_parse_number
  dq xt_dup, xt_fetch_char, xt_dup, xt_char_is_digit, xt_not
  branch0 .unsigned
  dq xt_dup, xt_lit, '-', xt_equal, xt_not
  branch0 .negative

  dq xt_dup, xt_lit, '+', xt_equal, xt_not
  branch0 .positive

.negative:
  dq xt_drop
  dq xt_inc, xt_string_parse_unsigned_number
  dq xt_dup
  branch0 .error
  dq xt_inc
  dq xt_swap, xt_neg, xt_swap, xt_exit
.positive:
  dq xt_drop
  dq xt_inc, xt_string_parse_unsigned_number
  dq xt_dup
  branch0 .error
  dq xt_inc, xt_exit
.unsigned:
  dq xt_drop, xt_string_parse_unsigned_number, xt_exit 
.error:
  dq xt_drop, xt_lit, 0, xt_exit

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

; save addr of new word in user_dict and prepare to a next word
colon ",", comma
  dq xt_here, xt_fetch, xt_store
  dq xt_here, xt_fetch, xt_lit, 8, xt_plus
  dq xt_here, xt_store, xt_exit 

; preparing for write system call and, explicitly, the call itself
colon "sys-write", sys_write
	TIMES 3 dq xt_push_to_r
	dq xt_lit, sys_write_no
	TIMES 3 dq xt_pop_from_r
	TIMES 3 dq xt_lit, 0
	dq xt_syscall , xt_drop, xt_exit
	
; prepare cell on the stack for sym
colon  "stack-cell", stack_cell
  dq xt_lit, 0, xt_forth_sp, xt_exit
	
; write char to output corresponding to the fd
; ( fd char -- )
colon "file-write-char", file_write_char
  dq xt_swap, xt_push_to_r, xt_push_to_r
  dq xt_stack_cell, xt_dup, xt_pop_from_r, xt_swap, xt_store_char
  dq xt_pop_from_r, xt_swap, xt_lit, 1, xt_sys_write, xt_drop, xt_drop, xt_exit
  
; prints carriage return (starts new line)
colon "print-cr", print_cr
  dq xt_lit, 1, xt_lit, 10, xt_file_write_char, xt_exit

; outputs string of count syms
; ( str count -- )
colon "print-string-counted", print_string_counted
  dq xt_push_to_r, xt_push_to_r, xt_lit, 1, xt_pop_from_r, xt_pop_from_r, xt_sys_write, xt_drop, xt_exit
  
; outputs the string
; ( str -- )
colon "print-string", print_string
  dq xt_dup, xt_string_length, xt_print_string_counted, xt_exit


; commands for initializing forth and for the interpretation process  

; adds word to dictionary (increments the dict length)
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
	
; checks the presence of branch commands before the current one
colon "forth-follows-branch", forth_follows_branch
    dq xt_here, xt_fetch, xt_lit, 8, xt_sub, xt_fetch
    dq xt_dup, xt_lit, xt_branch0, xt_equal, xt_swap
    dq xt_lit, xt_branch, xt_equal, xt_lor, xt_exit
  
; ( str -- )
; accepts word and tries to interpret it as a command, special command or a number
colon "forth-interpret", forth_interpret
.start:
    ; ( inbuff )
    dq xt_dup, xt_forth_find_word		; try find input word in dictionary
    branch0 .number						; if not found then maybe it's number?
    dq xt_swap, xt_drop, xt_forth_cfa	; if found - convert word address to XT of the word

    dq xt_state, xt_fetch
    branch0 .interpreting		; interpreting state?

    dq xt_dup, xt_forth_xt_is_immediate, xt_not
    branch0 .interpreting		; if immediate word - interpret it

.compiling:
    dq xt_comma, xt_exit
.interpreting:
    dq xt_execute, xt_exit

.number:
    dq xt_dup, xt_string_parse_number		; input is number?
    branch0 .no_word

    dq xt_swap, xt_drop						; number ?
    dq xt_state, xt_fetch					
    branch0 .exit							; interpretation state? then finish it

    dq xt_forth_follows_branch, xt_not
    branch0 .is_branch

.not_branch:
    dq xt_lit, xt_lit, xt_comma
.is_branch:									; if there was branch jump before => finishing
    dq xt_comma, xt_exit

.no_word:									; if there is no such word => output error message
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

; implementation of a word that will be executed on SIGSEGV
colon "trap", trap
    dq xt_lit, wh_trap + 9, xt_print_string, xt_print_cr, xt_exit	

; initialize forth machine
forth_init:
	dq xt_lit, 0, xt_forth_interpret_fd, xt_bye