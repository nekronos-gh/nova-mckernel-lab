#pragma once

#include "config.h"
#include "scheduler.h"

// Foward declaration
class Ec;

class Pd {
  public:
    Ec *caps[MAX_CAPS] = {}; // Capability table
    int set_cap(unsigned, Ec *);
    Ec *get_cap(unsigned);

    ALWAYS_INLINE
    static inline void *operator new(size_t) {
        return Kalloc::allocator.alloc(sizeof(Pd));
    }
};
