; When using .extern and .entry it is important to assemble with -x and -e

.extern UTILFUNC ; Defined in good3.as
.entry ARR

START:  mov #5, r1
        jsr UTILFUNC ; Defined in good3.as
        prn r1
        stop

ARR:    .data 3, 2, 1