; When using .extern and .entry it is important to assemble with -x and -e

.extern ARR ; Defined in good2.as
.entry UTILFUNC ; Is used in good2.as

UTILFUNC: inc r1
          lea ARR, r2
          rts
