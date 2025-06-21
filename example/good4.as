; Test immediate and relative addressing
START:  mov #10, r1
        add r1, &LOOP
        sub r2, r4
LOOP:   jmp &START
        stop
