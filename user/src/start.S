.text
.global __start
__start:
    # Before setting we need to map the user main stack
    # Number of pages
    mov $USER_STACK_PAGES, %eax
    mov %eax, mmap_frame + 8
    # Start address (Skip first page)
    mov $_user_stack_bottom, %ebx
    add $PAGE_SIZE, %ebx
    mov %ebx, mmap_frame + 12


    # Convention defined in include/syscall.h
    mov $mmap_frame, %eax
    mov %esp, %ecx
    mov $1f, %edx
    sysenter
1:

    mov $_user_stack_top, %esp

    # Call static constructors
    mov $__init_array_start, %ebx
1:
    cmp $__init_array_end, %ebx
    jz 2f
    call *(%ebx)
    add $4, %ebx
    jmp 1b
2:
    call main_func
    ud2

.data
.align 4
mmap_frame:
    .long 0                                 # SYS_MMAP
    .long 2                                 # argc
    .long 0                                 # argv[0] = n_pages 
    .long 0                                 # argv[1] = start_address 
    .long 0
