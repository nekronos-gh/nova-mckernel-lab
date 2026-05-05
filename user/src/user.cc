#include "stdio.h"

#define NORETURN __attribute__((noreturn))
#define EXTERN_C extern "C"

EXTERN_C NORETURN void main_func() {
    printf("Hello from user space: 0x%x\n", &main_func);
    while (1)
        ;
}
