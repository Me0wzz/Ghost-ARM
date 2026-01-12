.global _Reset
_Reset:
    LDR sp, =0x10000 // Initialize stack pointer
    BL c_entry // Call main entry point
stop:
    B stop  // Infinite loop


