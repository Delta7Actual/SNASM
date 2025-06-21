LOOP:       mov r1, r2       ; first definition of label LOOP
            add r3, r4

LOOP:       sub r2, r1       ; second definition of LOOP - illegal
            stop
