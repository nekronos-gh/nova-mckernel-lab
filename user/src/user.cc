#include "execution.h"
#include "ipc.h"
#include "stdio.h"
#include "symbols.h"

#define NORETURN __attribute__((noreturn))
#define EXTERN_C extern "C"

// WALL OF SHAME
// TODO: Starvation of low-priority ECs; implement aging or preemptive
// scheduling.
// TODO: Only one PD for now (root): extend to multiple execution contexts in
// different memory spaces.
// INFO: IF EC1 writes to EC2 and vice-versa, then Deadlock occurs.
// TODO: Implement IPC between PDs using portals and Scheduling Contexts
// --------------

static constexpr unsigned REDUCER_SLOT = 1;
static constexpr unsigned MAPPER0_SLOT = 2;
static constexpr unsigned MAPPER1_SLOT = 3;
static constexpr unsigned MAPPER2_SLOT = 4;
static constexpr unsigned MAIN_SLOT = 0; // main_func is the root EC
//
static constexpr unsigned NUM_MAPPERS = 3;

NORETURN void reducer() {
    printf("[user::reducer]\t started, waiting for %d mapped values\n",
           NUM_MAPPERS);

    unsigned sum = 0;
    for (unsigned i = 0; i < NUM_MAPPERS; ++i) {
        // Block until the next mapper writes to us
        int val = read();
        printf(
            "[user::reducer]\t received partial result %d (running sum = %d)\n",
            val, sum + static_cast<unsigned>(val));
        sum += static_cast<unsigned>(val);
        yield();
    }

    printf("[user::reducer]\t all partial results collected, total = %d\n",
           sum);

    // Send the final result back to main
    add_capability(MAIN_SLOT);
    write(MAIN_SLOT, sum);

    while (1) {
        yield();
    }
}

// Apply f(x) = x * x
NORETURN static void run_mapper(unsigned mapper_id) {
    printf("[user::mapper%u]\t started, waiting for input from main\n",
           mapper_id);

    // Receive one element from main_func
    int raw = read();

    unsigned mapped = static_cast<unsigned>(raw) * static_cast<unsigned>(raw);
    printf("[user::mapper%u]\t mapped value (%u^2) = %u\n", mapper_id,
           static_cast<unsigned>(raw), mapped);

    // Yield to let other ECs
    yield();

    // Send partial result to the reducer
    printf("[user::mapper%u]\t sending partial result %u to reducer\n",
           mapper_id, mapped);
    add_capability(REDUCER_SLOT);
    write(REDUCER_SLOT, mapped);

    while (1) {
        yield();
    }
}

// Mapper ids
NORETURN void mapper0() { run_mapper(0); }
NORETURN void mapper1() { run_mapper(1); }
NORETURN void mapper2() { run_mapper(2); }

EXTERN_C NORETURN void main_func() {
    printf("[user::main]\t IPC buffer at %p\n",
           reinterpret_cast<mword>(&_ipc_buffer_status));
    printf("[user::main]\t starting map-reduce demo\n");

    // Create reducer
    int r_slot = create_ec(reducer, /*priority=*/0, REDUCER_SLOT);
    if (r_slot < 0) {
        printf("[user::main]\t ERROR: failed to create reducer\n");
        while (1)
            ;
    }

    // Create mappers
    void (*mapper_funcs[NUM_MAPPERS])() = {mapper0, mapper1, mapper2};
    unsigned mapper_slots[NUM_MAPPERS] = {MAPPER0_SLOT, MAPPER1_SLOT,
                                          MAPPER2_SLOT};
    unsigned inputs[NUM_MAPPERS] = {5, 4, 1}; // 42!
    for (unsigned i = 0; i < NUM_MAPPERS; ++i) {
        int slot = create_ec(mapper_funcs[i], /*priority=*/0, mapper_slots[i]);
        if (slot < 0) {
            printf("[user::main]\t ERROR: failed to create mapper%u\n", i);
            while (1)
                ;
        }
        // Main must be able to write to all mappers
        add_capability(static_cast<unsigned>(slot));
        printf("[user::main]\t created mapper%u at cap_slot %d\n", i, slot);
    }
    // Send the values to the mappers
    for (unsigned i = 0; i < NUM_MAPPERS; ++i) {
        printf("[user::main]\t sending input %u to mapper%u (cap_slot %u)\n",
               inputs[i], i, mapper_slots[i]);
        write(mapper_slots[i], inputs[i]);
    }

    printf("[user::main]\t created reducer at cap_slot %d\n", r_slot);

    int result = read();
    printf("[user::main]\t --- final reduce result = %d ---\n", result);
    while (1)
        ;
}
