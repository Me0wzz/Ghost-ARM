.global _Reset
_Reset:
    LDR r0, = 0x101f1000
    MOV r1, #'X'
    STR r1, [r0]
stop:
    B stop  // Infinite loop


