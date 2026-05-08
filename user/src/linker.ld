OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH("i386")
ENTRY(__start)

PAGE_SIZE        = 0x1000;                              /* 4KB */
USER_STACK_PAGES = 2048;                                /* 8MB */
USER_STACK_SIZE  = (USER_STACK_PAGES + 1) * PAGE_SIZE;  /* +1 guard page */
HEAP_SIZE        = USER_STACK_SIZE * 2028;              /* 16GB */

PHDRS
    {
	data    PT_LOAD FLAGS(6); /* RW */
	text    PT_LOAD FLAGS(5); /* RX */
}

SECTIONS
    {
    . =  PAGE_SIZE + SIZEOF_HEADERS;
    . = ALIGN(4);

    .init_array : {
        __init_array_start = .;
        KEEP(*(.init_array))
        __init_array_end = .;
    } : data

    .data :
        {
        *(.data .data.*);
        // Define Heap
        . = ALIGN(PAGE_SIZE);
        _heap_f = .;
        . += HEAP_SIZE;
        _heap_e = .;

        // Define Stack with overflow page
        . = ALIGN(PAGE_SIZE);
        _stack_guard = .;
        . += PAGE_SIZE;
        . += USER_STACK_SIZE - PAGE_SIZE;
        . = ALIGN(PAGE_SIZE);
        stack_top = .;
    } : data

    .bss : {
        *(.bss .bss.*);
        *(COMMON);
    } : data
    .bss : {
        *(.bss .bss.*);
        *(COMMON);
    } : data

    .text : {
        . = ALIGN(PAGE_SIZE);
        *(.text .text.*);
    } : text

    .rodata : {
        *(.rodata .rodata.*);
    } : text

    /DISCARD/ : {
        *(.comment*)
        *(.eh_frame)
        *(.note*)
    }

}
