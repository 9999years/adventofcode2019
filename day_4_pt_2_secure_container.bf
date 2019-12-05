  0 "dd"5:+*** 2 "d":**+ 2 "d"5:+**+ 1 "d"*+ 1 5:+*+ 1+v
; ^            ^         ^           ^       ^       ^
v<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

                                          v3$<
                                      >>>># >^
                                >\:2/#^_:2%!|
            > ::5:+/5:+%\5:+%-!!|    >$1v<< v (not continue_run)
>  :3\>:5:+`|                   >\:2/|  v   v
      ^     # (top < 10)             >$0v   v
      ^<<<<<<<<<<<<<<<<<<<<<<<<<<<</+:5\<   \     >$1>>> ..@; OUTPUT
            >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>$:2/#^_2%!^
                                              (after break)

| ; up for 1, down for 0
_ ; left for 1, right for 0

; Check that there's two adjacent identical digits, not in a run (0 true, 1 false):
    ; - Duplicate top of stack
    ; - Push 3, swap top 2
    ;   - the 3 counts if we're in a digit run *and* if we've seen two digits
    ;     in the past
    ;     - bit 0: in a digit run? 0 for true, 1 for false
    ;       - to get bit 0, modulo by 2
    ;     - bit 1: seen two adjacent digits ever? 0 for true, 1 for false
    ;       - to get bit 1, divide by 2
    ;     - bit 2: seen two adjacent digits not in a run? 0 for true, 1 for false
    ;       - to get bit 2, divide by 4
    ; 2. If number < 10, go to 4.
    ; 3. Check if last 2 digits are the same
    ;   - If yes (0):
    ;     - Swap top of stack, check bit 0 (in a run)
    ;     - If bit 0 is 0, we're in a run; ignore these:
    ;       - Set top of stack to 
    ;     - If bit 0 is 1, we need to mark that we've seen 2 digits
    ;       - set top of stack to 0 (in a run, seen two digits)
    ;   - If no (1):
    ;     - 
    ; 4. Quitting
    ;   - Pop stack
    ;   - Check top

  2 "dd"5:+*** 3 "d":**+ 1 "d"5:+**+ 8 "d"*+ 3 5:+*+ 2+ "ddd"***v ; Input lower bound.
v<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
> 4 "dd"5:+*** 6 "d":**+ 8 "d"5:+**+ 1 "d"*+ 6 5:+*+ 8+\v ; Iterations.
v<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
v  >v                    ^+**"ddd"\-1\<< ; Add 1 to the password being tested.
>\:|#                                  ^
   >$:"ddd"**/."ddd"**%.@         >0v  ^
v<<\<         >!>>>>>>>>>>>>>>>>>>$>v  ^<
>:"ddd"**/v   ^                     v  $^
v<<<<<<<<<<   ^                     v >|^
>>>>:>::5:+`!#^_:5:+/5:+%\5:+%-!!!| v ^$^
     ^<<<<<<<<<<<<<<<<<<<<<<<</+:5< ! ^1^
   v<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<_1^+^
   v                           >1v    ^>^
   v          >>>>>>>>>>>>>>>>>$!>>>>>^
   >:>::5:+`!#^_:5:+/5:+%\5:+%`|
     ^<<<<<<<<<<<<<<<<<<<<</+:5<

; === Functions ===

; If top of stack is less than 10, push 0, otherwise push 1:
    9`

; Push last digit on top of stack to stack:
    :5:+%

; Push second-to-last digit on top of stack to stack:
    :5:+/5:+%

; Divide top of stack by 10:
    5:+/

; Check if last two digits are increasing (0 if true, 1 if false):
    ; - Duplicate top of stack.
    ; - Push second-to-last digit.
    ; - Swap top two elements of stack.
    ; - Replace top of stack with last digit.
    ; - gt: b = 10s place, a = 1s place, which means we push 1 if decreasing, 0
      ; if equal or increasing
    ::5:+/5:+%\5:+%`
  ; ^ dup      ^^^^ repl top w/ last digit
  ;  ^^^^^^^^^ push 2nd to last
  ;           ^ swp
  ;                ^ gt

; Check if all digits are increasing (0 if true, 1 if false):
    ; 1. Duplicate top of stack
    ; 2. If number < 10, push 0 and quit
        ; 1 => true (number < 10)
        ; 0 => false (number >= 10)
    ; 3. Check if last 2 digits are decreasing
      ; - If no (1), quit with 1 on top
      ; - If yes (0), divide top of stack by 10 and go to 2.
                               >1v
              >>>>>>>>>>>>>>>>>$!> ; Output
    :>::5:+`!#^_:5:+/5:+%\5:+%`|
     ^<<<<<<<<<<<<<<<<<<<<</+:5<

; Check if last two digits are the same (0 if true, 1 if false):
    ; - Duplicate top of stack.
    ; - Push second-to-last digit.
    ; - Swap top two elements of stack.
    ; - Replace top of stack with last digit.
    ; - Subtract
    ; - NOT-NOT
    ::5:+/5:+%\5:+%-!!

; Check that there's two adjacent identical digits (0 true, 1 false):
    ; - Duplicate top of stack
    ; 2. If number < 10, push 1 and quit
    ; 3. Check if last 2 digits are the same
    ;   - If yes (0), push 0 and quit
    ;   - If no (1), divide top of stack by 10 and go to 2
                                  >0v
              >!>>>>>>>>>>>>>>>>>>$>> ; Output
    :>::5:+`!#^_:5:+/5:+%\5:+%-!!!|
     ^<<<<<<<<<<<<<<<<<<<<<<<</+:5<

; Loop
    ; - Push context. (whatever data the loop needs)
    ;   Context: 12-digit number
    ;     - First 6 digits: current number being checked
    ;     - Last 6 digits:  number of valid passwords
    ; - Push iterations.
    ; 1. If iterations = 0, quit.
    ; 2. Otherwise, run loop.
    ; 3. Decrement iterations.
    ; 4. Go to 1.

; === Numeric constants ===
5:+       ; = 10
"d"       ; = 100
"d"5:+*   ; = 1000
"d":*     ; = 10000
"dd"5:+** ; = 100000
"ddd"**   ; = 1000000

; === Puzzle input ===
  2 "dd"5:+*** 3 "d":**+ 1 "d"5:+**+ 8 "d"*+ 3 5:+*+ 2+ .@ ; Input lower bound.
  7 "dd"5:+*** 6 "d":**+ 7 "d"5:+**+ 3 "d"*+ 4 5:+*+ 6+ .@ ; Input upper bound.
; ^            ^         ^           ^         ^     ^
; These entries are the digits of a 6-digit number.
; The given upper bound is 767346, but the last valid entry is 699999.
; End - start = 699999-231832 = 468167
