#pragma once
#include "heap.h"
#include "syscall.h"

static void clone(void (*func)(), unsigned priority) {
    void *stack = ProcessHeap::heap.alloc_stack();
    if (!stack)
        return;
    syscall(SYS_CLONE, reinterpret_cast<mword>(func),
            reinterpret_cast<mword>(stack), static_cast<mword>(priority));
}

static void yield() { syscall(SYS_YIELD); }
