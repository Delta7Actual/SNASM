mcro DOUBLE
    add r1, r1
mcroend

mcro QUADUP
    add r3, r3
    add r3, r3
mcroend

; Start of the program
MAIN: mov #5, r3
      DOUBLE
      cmp r2, #5
      stop
      QUADUP

TEST: .data -13, +4, 9
.entry MAIN ; Config main as entry