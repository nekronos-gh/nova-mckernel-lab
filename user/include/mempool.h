#pragma once
#include "types.h"
class UserMemPool {
  private:
    mword begin, end;

  public:
    static UserMemPool heap;

    UserMemPool(mword m_begin, mword m_end) : begin(m_begin), end(m_end) {}

    void *alloc_stack();
};
