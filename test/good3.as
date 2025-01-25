; Extern and entry symbols
.extern EXTERNAL_LABEL
.entry ENTRY_LABEL

        mov r3, EXTERNAL_LABEL
ENTRY_LABEL: add r5, r6
             stop
