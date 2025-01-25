; Program with labels and directives
START:  .data 5, -3, 15
        .string "hello"
        lea STR, r4
        inc r4
STR:    .data 10
        prn r4
        stop
