#pragma once
#include "mempool.h"
#include "syscall.h"

static int create_ec(void (*func)(), unsigned priority, unsigned cap_slot,
                     unsigned target_pd) {
    void *stack = UserMemPool::heap.alloc_stack();
    if (!stack)
        return -1;
    return syscall(SYS_CREATE_EC, reinterpret_cast<mword>(func),
                   reinterpret_cast<mword>(stack), static_cast<mword>(priority),
                   static_cast<mword>(cap_slot), static_cast<mword>(target_pd));
}

static void yield() { syscall(SYS_YIELD); }

static void block() { syscall(SYS_BLOCK); }

static void block(unsigned tid) { syscall(SYS_BLOCK, tid); }

static void unblock() { syscall(SYS_UNBLOCK); }

static void unblock(unsigned tid) { syscall(SYS_UNBLOCK, tid); }
