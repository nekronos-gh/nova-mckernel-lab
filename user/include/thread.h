#pragma once
#include "heap.h"
#include "syscall.h"

static unsigned clone(void (*func)(), unsigned priority) {
    void *stack = ProcessHeap::heap.alloc_stack();
    if (!stack)
        return 0;
    return syscall(SYS_CLONE, reinterpret_cast<mword>(func),
                   reinterpret_cast<mword>(stack),
                   static_cast<mword>(priority));
}

static void yield() { syscall(SYS_YIELD); }

static void block() { syscall(SYS_BLOCK); }

static void block(unsigned tid) { syscall(SYS_BLOCK, tid); }

static void unblock() { syscall(SYS_UNBLOCK); }

static void unblock(unsigned tid) { syscall(SYS_UNBLOCK, tid); }
