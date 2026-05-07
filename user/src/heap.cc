
#include "heap.h"
#include "stdio.h"

extern char _heap_f, _heap_e;

ProcessHeap ProcessHeap::heap(reinterpret_cast<mword>(&_heap_f),
                              reinterpret_cast<mword>(&_heap_e));

void *ProcessHeap::alloc_stack() {
    if (begin + STACK_SIZE > end) {
        printf("Heap full\n");
        return nullptr;
    }

    void *p = reinterpret_cast<void *>(begin);

    begin += STACK_SIZE;
    return p;
}
