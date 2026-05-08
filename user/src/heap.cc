
#include "heap.h"
#include "stdio.h"
#include "symbols.h"

ProcessHeap ProcessHeap::heap(reinterpret_cast<mword>(&_heap_start),
                              reinterpret_cast<mword>(&_heap_end));

void *ProcessHeap::alloc_stack() {
    if (begin + USER_STACK_SIZE > end) {
        printf("Heap full begin 0x%x: start 0x%x:\n", begin, end);
        return nullptr;
    }

    void *p = reinterpret_cast<void *>(begin);

    // Return end address of the stack allocated
    begin += USER_STACK_SIZE;
    return p;
}
