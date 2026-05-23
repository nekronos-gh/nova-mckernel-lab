#include "stdio.h"
#include "thread.h"

#define NORETURN __attribute__((noreturn))
#define EXTERN_C extern "C"

NORETURN void thread_1() {
    printf("Hello from Thread 1!\n", &thread_1);
    yield();
    yield();
    while (1)
        ;
}

NORETURN void thread_2() {
    printf("Hello from Thread 2!\n", &thread_2);
    yield();
    while (1)
        ;
}

EXTERN_C NORETURN void main_func() {
    printf("Hello from user space!\n", &main_func);

    clone(&thread_1);
    yield();
    clone(&thread_2);
    yield();

    printf("Back on track...\n");
    while (1)
        ;
}
