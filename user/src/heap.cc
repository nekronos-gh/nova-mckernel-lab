
#include "heap.h"
#include "stdio.h"
#include "symbols.h"
#include "syscall.h"

ProcessHeap ProcessHeap::heap(reinterpret_cast<mword>(&_heap_f),
                              reinterpret_cast<mword>(&_heap_e));

void *ProcessHeap::alloc_stack() {
    if (begin + USER_STACK_SIZE > end) {
        printf("Heap full begin 0x%x: start 0x%x:\n", begin, end);
        return nullptr;
    }

    // First page of a stack is a Stack Overflow Sentinel
    syscall(SYS_MMAP, USER_STACK_PAGES, begin + PAGE_SIZE);
    begin += USER_STACK_SIZE;

    // Return end address of the stack allocated
    return reinterpret_cast<void *>(begin);
}
