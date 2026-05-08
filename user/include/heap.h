#pragma once

#include "types.h"

#define PAGE_BITS 12
#define PAGE_SIZE (1 << PAGE_BITS)
// 2 Pages (4KB) per stack
// 1 Unmapped page for stack overflow
#define STACK_SIZE ((PAGE_SIZE << 1) + PAGE_SIZE)

class ProcessHeap {
  private:
    mword begin, end;

  public:
    static ProcessHeap heap;

    ProcessHeap(mword h_begin, mword h_end) : begin(h_begin), end(h_end) {}

    void *alloc_stack();
};
