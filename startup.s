.global _Reset
.global enable_irq
.equ MODE_FIQ, 0xD1
.equ MODE_IRQ, 0xD2 // Interrupt mode
.equ MODE_SVC, 0xD3 // Supervisor mode

_Reset:
    LDR pc, =reset_handler
    LDR pc, =undefined_handler
    LDR pc, =swi_handler 
    LDR pc, =prefetch_handler
    LDR pc, =data_handler
    LDR pc, =unused_handler
    LDR pc, =irq_handler
    LDR pc, =fiq_handler

reset_handler:
    MOV r0, #0x10000
    MOV r1, #0x00000
    MOV r2, #0x100

copy_loop:
copy_loop:
    LDR r3, [r0], #4 
    STR r3, [r1], #4
    SUBS r2, r2, #4  
    BNE copy_loop
    MSR cpsr_c, #MODE_IRQ // Switch to IRQ mode
    LDR sp, =0x8000 // Set up stack pointer for IRQ mode
    MSR cpsr_c, #MODE_SVC // Switch to Supervisor mode
    LDR sp, =0x10000 // Set up stack pointer for SVC mode

    BL c_entry // Call main entry point
    B . // Infinite loop

irq_handler:
    SUB lr, lr, #4 // Return address correction
    STMFD sp!, {r0-r12, lr} // Save registers
    BL c_irq_handler // Call C IRQ handler
    LDMFD sp!, {r0-r12, pc}^ // Restore registers and return

undefined_handler:
    B .
swi_handler:
    B .
prefetch_handler:
    B .
data_handler:
    B .
unused_handler:
    B .
fiq_handler:
    B .

enable_irq:
    MRS r0, cpsr
    BIC r0, r0, #0x80
    MSR cpsr_c, r0
    MOV pc, lr
