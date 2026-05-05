#pragma once

#include "types.h"

enum class SyscallNum : uint8 {
    SYS_DUMP = 0,
    SYS_PRINT = 1,
    SYS_CLONE = 2,
    MAX_SYSCALL
};

struct syscall_frame {
    SyscallNum num;
    unsigned argc;
    unsigned argv[3]; // Max three arguments
};

class Syscall {
  public:
    virtual void handle(syscall_frame *f) = 0;
};

extern Syscall *syscall_table[static_cast<unsigned>(SyscallNum::MAX_SYSCALL)];
