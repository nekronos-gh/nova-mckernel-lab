#pragma once

#include "ec.h"
#include "types.h"

enum SyscallNum : uint8 {
    SYS_MMAP = 0,
    SYS_DUMP = 1,
    SYS_PRINT = 2,
    SYS_CREATE_EC = 3,
    SYS_YIELD = 4,
    SYS_BLOCK = 5,
    SYS_UNBLOCK = 6,
    MAX_SYSCALL
};

// POD
struct syscall_frame {
    SyscallNum num;
    unsigned argc;
};

struct syscall_mmap : public syscall_frame {
    unsigned n_pages;
    mword virt_addr;
};

struct syscall_dump : public syscall_frame {
    // No additional fields
};

struct syscall_print : public syscall_frame {
    const char *fmt;
    va_list args;
};

struct syscall_create_ec : public syscall_frame {
    mword eip_val;
    mword esp_val;
    unsigned priority_val;
    unsigned capability_val;

    mword eip() const { return eip_val; }
    mword esp() const { return esp_val; }
    unsigned priority() const { return priority_val; }
    unsigned capability() const { return capability_val; }
};

struct syscall_block : public syscall_frame {
    unsigned capability;
};

struct syscall_unblock : public syscall_frame {
    unsigned capability;
};

class Syscall {
  public:
    virtual void handle(syscall_frame *f) = 0;
};

extern Syscall *syscall_table[static_cast<unsigned>(SyscallNum::MAX_SYSCALL)];
