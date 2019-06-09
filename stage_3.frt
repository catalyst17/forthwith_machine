: here forth-here @ ;

: IMMEDIATE forth-last-word @ forth-cfa 1 - dup c@ 1 or swap c! ;

: do here ; IMMEDIATE
: while ' 0branch , , ; IMMEDIATE

: ( do 0 file-read-char drop 41 = while ; IMMEDIATE

( commands defined above let us to write comments now )

( via "here" we define kind of "labels" where we need 
	to jump from "here 0" places in our code
	in order to implement branching )

: if ' 0branch , here 0  , ; IMMEDIATE
: else ' branch , here 0 , swap here swap !  ; IMMEDIATE
: then here swap ! ; IMMEDIATE

: for 
      ' swap ,
      ' >r , ( storing iterator and ... )
      ' >r , ( ... loop limit in rstack in order not to lose them )
( checking if limit's been reached )
here  ' r> , 
      ' r> , 
      ' 2dup , 
      ' >r , 
      ' >r , 
      ' < ,  
      ' 0branch ,  
	  here 0 , ( jump to "endfor" code, finishing loop )
swap ; IMMEDIATE

: endfor
      ' r> ,            ( popping iterator from rstack ... )
      ' lit , 1 ,       ( ... incrementing ... )
      ' + ,             ( ... it ... )
      ' >r ,            ( ... and pushing back )
      ' branch ,    	( continue loop, going back to "for" code )
( complete the loop and drop auxiliary variables )
, here swap !
      ' r> ,
      ' drop ,
      ' r> ,
      ' drop ,
; IMMEDIATE
