#include "stdio.h"
#include "thread.h"

#define NORETURN __attribute__((noreturn))
#define EXTERN_C extern "C"

// Priority 1 (high): run before priority 0 ECs; round-robin with each other.
// Priority 0 (low):  only run when the priority-1 queue is empty.

NORETURN void hi_thread_1() {
    printf("hi1: tick 1\n");
    yield();
    printf("hi1: tick 2\n");
    yield();
    while (1)
        ;
}

NORETURN void hi_thread_2() {
    printf("hi2: tick 1\n");
    yield();
    printf("hi2: tick 2\n");
    yield();
    while (1)
        ;
}

NORETURN void lo_thread_1() {
    printf("lo1: running\n");
    while (1)
        ;
}

NORETURN void lo_thread_2() {
    printf("lo2: running\n");
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
    // TODO: Confirm intended behavior, "main: back" is never reached because
    // the hi threads stop yielding, holding the CPU indefinitely (cooperative
    // scheduling, no preemption).
    printf("main: back\n");
    while (1)
        ;
}
