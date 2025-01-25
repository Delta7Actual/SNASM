mcro DOUBLE
    add r1, r1
mcroend

; Start of the program
MAIN: mov r2, r3
      DOUBLE
      cmp r2, #5
      stop
