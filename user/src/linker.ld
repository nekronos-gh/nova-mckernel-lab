OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH("i386")
ENTRY(__start)

PAGE_SIZE = 0x1000;                                     /* 4KB */
USER_STACK_PAGES = 2048;                                /* 8MB */
USER_STACK_SIZE = (USER_STACK_PAGES + 1) * PAGE_SIZE;   /* +1 guard page */
HEAP_SIZE = USER_STACK_SIZE * 16;              

MAX_CAPS = 8;
BUFFER_SIZE = 4;
BUFFER_FREE = MAX_CAPS + 1;


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
        /* Using shared memory for IPC */
        _ipc_buffer_status = .;
        . += 4 * MAX_CAPS;
        _ipc_buffer = ALIGN(4);                                            
        . += BUFFER_SIZE * MAX_CAPS;

        /* Data & BSS */
        *(.data .data.*);
        *(.bss .bss.*);
        . = ALIGN(PAGE_SIZE);
    } : data 

    /* Heap section */
    /* Clone stacks */
    .heap (NOLOAD) : {
        _heap_f = .;
        . += HEAP_SIZE;
        _heap_e = .;
    }

    /* Main stack outside of heap */
    .stack (NOLOAD) : {
        . = ALIGN(PAGE_SIZE);
        _user_stack_bottom = .;
        . += USER_STACK_SIZE;
        _user_stack_top = .;
    }
}
