.entry PRINTF

PRINTF: mov r1, r2
        mov #1, r1
        mov #8, r0
        int
        rts