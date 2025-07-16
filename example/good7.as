.entry START
.extern PRINTF

STRING: .string "hello world"

START:  lea STRING, r1
        jsr PRINTF
        stop