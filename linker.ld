ENTRY(_Reset)
SECTIONS
{
    . = 0x10000;
    .startup : { startup.o(.text) }
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { 
        __bss_start = .;
        *(.bss) 
        __bss_end = .;
    }
    . = . + 0x1000;
    stack_top = .;
}