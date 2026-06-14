#pragma once
#include "syscall.h"

static int ipc_send(unsigned cap_slot, mword value) {
    return static_cast<int>(
        syscall(SYS_IPC_SEND, static_cast<mword>(cap_slot), value));
}

static int ipc_recv() { return static_cast<int>(syscall(SYS_IPC_RECV)); }
