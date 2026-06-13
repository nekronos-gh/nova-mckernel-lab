#include "execution.h"
#include "stdio.h"
#include "symbols.h"

#define BUFFER_FREE MAX_CAPS + 1

int write(unsigned cap_slot, unsigned message) {
    // Check Capability, if not return
    if (!check_capability(cap_slot))
        return -1;
    // Busy wait
    bool sent = 0;
    while (reinterpret_cast<unsigned>(_ipc_buffer_status) != cap_slot) {
        yield();
    }
    // Write and transfer execution to receiver
    _ipc_buffer = message;
    unblock(cap_slot);
    yield();
    return 0;
}

int read() {
    // Check Capability, if not return
    // Busy wait
    bool read = 0;
    unsigned cap_slot = get_cap_slot();

    // If the buffer is not free, we are not
    while (reinterpret_cast<unsigned>(_ipc_buffer_status) != BUFFER_FREE) {
        yield();
    }
    _ipc_buffer_status = cap_slot;
    block(cap_slot);
    // Result should be on the address
    int result = static_cast<int>(_ipc_buffer);
    // Clean the data
    _ipc_buffer = 0x0;
    _ipc_buffer_status = BUFFER_FREE;
    return result;
}
