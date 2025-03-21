mcro DOUBLE
    add r1, r1
mcroend

mcro QUADUP
    add r3, r3
    add r3, r3
mcroend

; Start of the program
MAIN: mov r2, r3
      DOUBLE
      cmp r2, #5
      stop
      QUADUP

TEST: mov r1, r3