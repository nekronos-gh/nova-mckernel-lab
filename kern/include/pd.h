#pragma once

#include "scheduler.h"

// Foward declaration
class Ec;

class Pd {
    static constexpr unsigned MAX_CAPS =
        8; // For now lets stick to 8 ECs per PD
  public:
    Ec *caps[MAX_CAPS] = {}; // Capability table
    int set_cap(unsigned, Ec *);
    Ec *get_cap(unsigned);

    ALWAYS_INLINE
    static inline void *operator new(size_t) {
        return Kalloc::allocator.alloc(sizeof(Pd));
    }
};
