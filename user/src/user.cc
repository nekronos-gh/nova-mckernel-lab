#include "stdio.h"

#define NORETURN __attribute__((noreturn))
#define EXTERN_C extern "C"

NORETURN void thread_1() {
    printf("Hello from thread 1\n");
    while (1)
        ;
}

EXTERN_C NORETURN void main_func() {
    printf("Hello from main function\n");
    while (1)
        ;
}
