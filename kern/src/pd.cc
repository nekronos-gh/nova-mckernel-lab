#include "pd.h"

int Pd::set_cap(unsigned slot, Ec *ec) {
    if (slot >= MAX_CAPS)
        return -1; // out of bounds
    if (caps[slot] != nullptr)
        return -1; // slot already occupied
    caps[slot] = ec;
    return static_cast<int>(slot);
}

Ec *Pd::get_cap(unsigned slot) {
    if (slot >= MAX_CAPS) {
        return nullptr;
    }
    return caps[slot];
}

int Pd::add_child(Pd *child) {
    for (unsigned i = 0; i < MAX_CAPS; ++i) {
        if (children[i] == nullptr) {
            children[i] = child;
            return static_cast<int>(i);
        }
    }
    return -1; // no free slot
}

bool Pd::is_ancestor_of(Pd *other) const {
    // Walk other's parent chain upward
    // if we appear, we are an ancestor
    Pd *p = other->parent;
    while (p) {
        if (p == this)
            return true;
        p = p->parent;
    }
    return false;
}
