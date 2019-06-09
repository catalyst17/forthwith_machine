: here forth-here @ ;

: IMMEDIATE forth-last-word @ forth-cfa 1 - dup c@ 1 or swap c! ;

: do here ; IMMEDIATE
: while ' 0branch , , ; IMMEDIATE

: ( do 0 file-read-char drop 41 = while ; IMMEDIATE

( commands defined above let us to write comments now )
