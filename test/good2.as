STRING: .string "hello world!"
mov #12, r2

LOOP: prn r1
dec r2
bne &END

END: stop