#include "heap.h"
#include "syscall.h"

inline void clone(void *addr) {

    void *stack = ProcessHeap::heap.alloc_stack();
    if (!stack)
        return;
    syscall(SYS_CLONE, reinterpret_cast<mword>(addr),
            reinterpret_cast<mword>(stack));
}
