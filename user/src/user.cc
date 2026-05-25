#include "stdio.h"
#include "thread.h"

#define NORETURN __attribute__((noreturn))
#define EXTERN_C extern "C"

// Priority 1 (high): run before priority 0 ECs; round-robin with each other.
// Priority 0 (low):  only run when the priority-1 queue is empty.
// FIX: Starvation of low-priority ECs; implement aging or preemptive
// scheduling.

NORETURN void hi_thread_1() {
    // 1st thread to run, should print first
    printf("A man who ");
    yield();
    // 5th event to run
    printf("\n― Arturo Pérez-Reverte");
    yield();
    printf("o////||::===========================-\n");
    printf("     []\n");
    yield();
    while (1)
        ;
}

NORETURN void hi_thread_2() {
    // 2nd thread to run, should print second
    printf("seeks revenge should ");
    yield();
    // 6th event to run
    printf(", Captain Alatriste\n");
    yield();
    printf("\n     []\n");
    yield();
    while (1)
        ;
}

NORETURN void lo_thread_1() {
    // 3rd thread to run, should print third
    printf("dig two graves.\n");
    yield();
    while (1)
        ;
}

NORETURN void lo_thread_2() {
    // 4th thread to run
    printf("One for his enemy And one for himself.\n");
    unblock(); // Unblock all
    yield();
    while (1)
        ;
}

EXTERN_C NORETURN void main_func() {
    printf("\n----------------------\n\n");
    unsigned tid1 = clone(hi_thread_1, 0);
    unsigned tid2 = clone(hi_thread_2, 0);
    clone(lo_thread_1, 1);
    clone(lo_thread_2, 1);
    // Execute higher priority threads first
    yield();

    // Disable higher priority threads to let lower priority threads run
    block(tid1);
    block(tid2);
    block(); // Block self to yield to lower priority threads

    // All are unblocked at this point due to lo 2 unblock, and the higher
    // priority threads will run first
    block(tid1);
    yield();
    unblock(tid1);
    block(tid2);
    yield();

    printf("\n----------------------\n");

    while (1)
        ;
}
