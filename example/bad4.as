.extern VAR          ; label VAR declared extern
.entry VAR           ; but also declared as entry in same file - conflict

        mov r1, VAR
        stop
