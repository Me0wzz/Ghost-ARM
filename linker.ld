ENTRY(_Reset)
SECTIONS
{
    . = 0x10000;
    .text : { *(.text) }
}