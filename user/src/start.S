.text
.global __start
__start:
    mov $stack_top, %esp

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
