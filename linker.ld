ENTRY(_Reset)
SECTIONS
{
    /* QEMU ARM start address */
    . = 0x10000;
    /* Code and Constants */
    .startup : { startup.o(.text) }
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    /* Data and BSS */
    .data : { *(.data) }
    .bss : { 
        __bss_start = .;
        *(.bss) 
        __bss_end = .;
    }
    /* Stack */
    . = ALIGN(8);
    . = . + 0x1000;
    . = ALIGN(8);
    stack_top = .;
}