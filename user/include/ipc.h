#include "execution.h"
#include "symbols.h"

#define BUFFER_FREE 0
#define BUFFER_BUSY 1

int write(unsigned cap_slot, unsigned message) {
    // Check Capability, if not return
    if (!check_capability(cap_slot))
        return -1;
    // Busy wait
    bool sent = 0;
    while (reinterpret_cast<unsigned>(_ipc_buffer_status[cap_slot]) ==
           BUFFER_BUSY) {
        yield();
    }
    // Write and transfer execution to receiver
    _ipc_buffer_status[cap_slot] = BUFFER_BUSY;
    _ipc_buffer[cap_slot] = message;
    unblock(cap_slot);
    yield();
    return 0;
}

int read() {
    // Check Capability, if not return
    // Busy wait
    bool read = 0;
    unsigned cap_slot = get_cap_slot();

    // Wait until there is data in the buffer
    while (reinterpret_cast<unsigned>(_ipc_buffer_status[cap_slot]) ==
           BUFFER_FREE) {
        block();
    }
    // Result should be on the address
    int result = static_cast<int>(_ipc_buffer[cap_slot]);
    // Clean the data
    _ipc_buffer[cap_slot] = 0x0;
    _ipc_buffer_status[cap_slot] = BUFFER_FREE;
    return result;
}
