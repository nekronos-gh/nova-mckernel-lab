#pragma once

#include "types.h"

class ProcessHeap {
  private:
    mword begin, end;

  public:
    static ProcessHeap heap;

    ProcessHeap(mword h_begin, mword h_end) : begin(h_begin), end(h_end) {}

    void *alloc_stack();
};
