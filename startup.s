.global _Reset
_Reset:
    LDR sp, =stack_top // Initialize stack pointer
    BL c_entry // Call main entry point
    B . // Infinite loop after main returns

