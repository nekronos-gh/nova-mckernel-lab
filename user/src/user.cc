#include "execution.h"
#include "stdio.h"

#define NORETURN __attribute__((noreturn))
#define EXTERN_C extern "C"

// Priority 1 (high): run before priority 0 ECs; round-robin with each other.
// Priority 0 (low):  only run when the priority-1 queue is empty.
// TODO: Starvation of low-priority ECs; implement aging or preemptive
// scheduling.
// TODO:: Only one PD for now (root): extend to multiple execution contexts in
// different memory spaces.

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
    printf("One for his enemy and one for himself.\n");
    unblock(); // Unblock all
    yield();
    while (1)
        ;
}

EXTERN_C NORETURN void main_func() {
    printf("\n----------------------\n\n");

    // Create Execution contexts
    int cap_slot1 = create_ec(hi_thread_1, 0, 0);
    int cap_slot2 = create_ec(hi_thread_2, 0, 1);
    create_ec(lo_thread_1, 1, 2);
    create_ec(lo_thread_2, 1, 3);

    // Execute higher priority threads first
    yield();

    // Disable higher priority threads to let lower priority threads run
    block(cap_slot1);
    block(cap_slot2);
    block(); // Block self to yield to lower priority threads

    // All are unblocked at this point due to lo 2 unblock, and the higher
    // priority threads will run first
    block(cap_slot1);
    yield();
    unblock(cap_slot1);
    block(cap_slot2);
    yield();

    printf("\n----------------------\n");

    // Capability test: slot 0 (hi_thread_1)
    if (check_capability(0))
        printf("cap slot 0 before grant: allowed\n");
    else
        printf("cap slot 0 before grant: not allowed\n");

    add_capability(0);

    if (check_capability(0))
        printf("cap slot 0 after grant:  allowed\n");
    else
        printf("cap slot 0 after grant:  not allowed\n");

    while (1)
        ;
}
