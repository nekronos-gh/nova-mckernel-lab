#pragma once

#include "types.h"

enum class SyscallNum : uint8 {
    SYS_DUMP = 0,
    SYS_PRINT = 1,
    SYS_CLONE = 2,
    SYS_YIELD = 3,
    MAX_SYSCALL
};

struct syscall_frame {
    SyscallNum num;
    unsigned argc;
    unsigned argv[3]; // Max three arguments
};

struct syscall_clone : public syscall_frame {
    mword ip() { return argv[0]; }
    mword sp() { return argv[1]; }
};

class Syscall {
  public:
    virtual void handle(syscall_frame *f) = 0;
};

extern Syscall *syscall_table[static_cast<unsigned>(SyscallNum::MAX_SYSCALL)];
