OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH("i386")
ENTRY(__start)

PAGE_SIZE = 0x1000;                                     /* 4KB */
USER_STACK_PAGES = 2048;                                /* 8MB */
USER_STACK_SIZE = (USER_STACK_PAGES + 1) * PAGE_SIZE;   /* +1 guard page */
HEAP_SIZE = USER_STACK_SIZE * 16;              

PHDRS
    {
    data    PT_LOAD FLAGS(6); /* RW */
    text    PT_LOAD FLAGS(5); /* RX */
}

SECTIONS
    {
    . = PAGE_SIZE + SIZEOF_HEADERS; 

    .text : {
        *(.text .text.*);
        *(.rodata .rodata.*)                                     
    } : text

    .data :
        {
        . = ALIGN(4);                                            

        /* Data & BSS */
        __init_array_start = .;
        KEEP(*(.init_array))
        __init_array_end = .;
        *(.data .data.*);
        *(.bss .bss.*);
        . = ALIGN(PAGE_SIZE);
        . += PAGE_SIZE;
    } : data 

    /* Heap section */
    _heap_f = ALIGN(PAGE_SIZE);                                         
    . += HEAP_SIZE;
    _heap_e = .;

    /* Main stack outside of heap*/
    . = ALIGN(PAGE_SIZE);
    . += USER_STACK_SIZE;
    stack_top = .;
}
