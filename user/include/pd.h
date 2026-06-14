#pragma once
#include "syscall.h"

static int create_pd() { return static_cast<int>(syscall(SYS_CREATE_PD)); }

static int delegate_cap(unsigned child_idx, unsigned src_slot,
                        unsigned dst_slot) {
    return static_cast<int>(
        syscall(SYS_DELEGATE_CAP, static_cast<mword>(child_idx),
                static_cast<mword>(src_slot), static_cast<mword>(dst_slot)));
}
