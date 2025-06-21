; Designed to work together with good3.as

.extern UTILFUNC
.entry START

START:  mov #5, r1
        jsr UTILFUNC
        prn r1
        stop

; Some local data
ARR:    .data 3, 2, 1
