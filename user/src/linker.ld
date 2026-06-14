OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH("i386")
ENTRY(__start)

PAGE_SIZE = 0x1000;                                     /* 4KB */
USER_STACK_PAGES = 2048;                                /* 8MB */
USER_STACK_SIZE = (USER_STACK_PAGES + 1) * PAGE_SIZE;   /* +1 guard page */
HEAP_SIZE = USER_STACK_SIZE * 16;              

PHDRS
    {
    text    PT_LOAD FLAGS(5); /* RX */
    data    PT_LOAD FLAGS(6); /* RW */
}

SECTIONS
    {
    . = PAGE_SIZE + SIZEOF_HEADERS; 

    .text : {
        *(.text .text.*);
        *(.rodata .rodata.*)                                     
        . = ALIGN(4);
        __init_array_start = .;
        KEEP(*(.init_array))
        __init_array_end = .;
    } : text

    .data :
        {
        . = ALIGN(PAGE_SIZE);                                            
        /* Data & BSS */
        *(.data .data.*);
        *(.bss .bss.*);
        . = ALIGN(PAGE_SIZE);
    } : data 

    /* Mempool section */
    /* Clone stacks */
    .heap (NOLOAD) : {
        _mempool_f = .;
        . += HEAP_SIZE;
        _mempool_e = .;
    }

    /* Main stack outside of mempool */
    .stack (NOLOAD) : {
        . = ALIGN(PAGE_SIZE);
        _user_stack_bottom = .;
        . += USER_STACK_SIZE;
        _user_stack_top = .;
    }
}
