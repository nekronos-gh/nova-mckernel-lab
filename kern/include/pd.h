#pragma once

#include "config.h"
#include "scheduler.h"

// Foward declaration
class Ec;

class Pd {
  public:
    Ec *caps[MAX_CAPS] = {};     // Capability table
    Pd *parent;                  // nullptr for root PD
    Pd *children[MAX_CAPS] = {}; // child PDs

    explicit Pd(Pd *p = nullptr) : parent(p) {}

    int set_cap(unsigned, Ec *);
    Ec *get_cap(unsigned);

    int add_child(Pd *child);

    bool is_ancestor_of(Pd *other) const;

    ALWAYS_INLINE
    static inline void *operator new(size_t) {
        return Kalloc::allocator.alloc(sizeof(Pd));
    }
};
