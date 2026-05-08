#pragma once

#include "types.h"

class ProcessHeap {
  private:
    mword begin, end;

  public:
    static ProcessHeap heap;
    uint32 user_stack_size;

    ProcessHeap(mword h_begin, mword h_end) : begin(h_begin), end(h_end) {
        user_stack_size = h_begin - h_end;
    }

    void *alloc_stack();
};
