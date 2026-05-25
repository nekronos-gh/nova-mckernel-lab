#include "stdio.h"
#include "thread.h"

#define NORETURN __attribute__((noreturn))
#define EXTERN_C extern "C"

// Priority 1 (high): run before priority 0 ECs; round-robin with each other.
// Priority 0 (low):  only run when the priority-1 queue is empty.
// FIX: Starvation of low-priority ECs; implement aging or preemptive
// scheduling.

NORETURN void hi_thread_1() {
    printf("Ping!\n");
    yield();
    printf("Ping!\n");
    yield();
    while (1)
        ;
}

NORETURN void hi_thread_2() {
    printf("Pong!\n");
    yield();
    printf("Pong!\n");
    yield();
    while (1)
        ;
}

NORETURN void lo_thread_1() {
    while (1)
        ;
}

NORETURN void lo_thread_2() {
    while (1)
        ;
}

EXTERN_C NORETURN void main_func() {
    printf("main: start\n");
    clone(hi_thread_1, 1);
    clone(hi_thread_2, 1);
    clone(lo_thread_1, 0);
    clone(lo_thread_2, 0);
    yield();
    // TODO: Implement preemptive scheduling
    printf("main: back\n");
    while (1)
        ;
}
