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
