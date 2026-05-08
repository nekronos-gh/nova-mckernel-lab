
#include "heap.h"
#include "stdio.h"

extern char _heap_f, _heap_e;

ProcessHeap ProcessHeap::heap(reinterpret_cast<mword>(&_heap_f),
                              reinterpret_cast<mword>(&_heap_e));

void *ProcessHeap::alloc_stack() {
    if (begin + STACK_SIZE > end) {
        printf("Heap full begin 0x%x: start 0x%x:\n", begin, end);
        return nullptr;
    }

    void *p = reinterpret_cast<void *>(begin);

    // Return end address of the stack allocated
    begin += STACK_SIZE;
    return p;
}
