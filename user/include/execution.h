#pragma once
#include "heap.h"
#include "syscall.h"

static int create_ec(void (*func)(), unsigned priority, unsigned cap_slot) {
    void *stack = ProcessHeap::heap.alloc_stack();
    if (!stack)
        return -1;
    return syscall(SYS_CREATE_EC, reinterpret_cast<mword>(func),
                   reinterpret_cast<mword>(stack), static_cast<mword>(priority),
                   static_cast<mword>(cap_slot));
}

static void yield() { syscall(SYS_YIELD); }

static void block() { syscall(SYS_BLOCK); }

static void block(unsigned tid) { syscall(SYS_BLOCK, tid); }

static void unblock() { syscall(SYS_UNBLOCK); }

static void unblock(unsigned tid) { syscall(SYS_UNBLOCK, tid); }

static bool check_capability(unsigned cap_slot) {
    return syscall(SYS_CHECK_CAP, cap_slot) != 0;
}

static void add_capability(unsigned cap_slot) {
    syscall(SYS_ADD_CAP, cap_slot);
}
