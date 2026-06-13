#include "execution.h"
#include "ipc.h"
#include "stdio.h"
#include "symbols.h"

#define NORETURN __attribute__((noreturn))
#define EXTERN_C extern "C"

// TODO: Starvation of low-priority ECs; implement aging or preemptive
// scheduling.
// TODO: Only one PD for now (root): extend to multiple execution contexts in
// different memory spaces.
// INFO: IF EC1 writes to EC2 and vice-versa, then Deadlock occurs.

NORETURN void th1() {
    // 4th thread to run
    printf("Th1 Recieved %d\n", read());
    add_capability(0);
    write(static_cast<unsigned>(0), 42);
    while (1)
        ;
}

EXTERN_C NORETURN void main_func() {

    // Create Execution contexts
    printf("Buffer %p", _ipc_buffer_status);
    int cap_slot1 = create_ec(th1, 0, 1);
    add_capability(cap_slot1);

    write(static_cast<unsigned>(cap_slot1), 42);
    printf("Main Recieved %d\n", read());

    while (1)
        ;
}
