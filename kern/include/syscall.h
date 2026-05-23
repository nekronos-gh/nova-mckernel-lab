#pragma once

#include "ec.h"
#include "types.h"

enum SyscallNum : uint8 {
    SYS_MMAP = 0,
    SYS_DUMP = 1,
    SYS_PRINT = 2,
    SYS_CLONE = 3,
    SYS_YIELD = 4,
    MAX_SYSCALL
};

struct syscall_frame {
    SyscallNum num;
    unsigned argc;
    unsigned argv[3]; // Max three arguments
};

struct syscall_clone : public syscall_frame {
    mword eip() { return argv[0]; }
    mword esp() { return argv[1]; }
    unsigned priority() { return argv[2]; }
};

class Syscall {
  public:
    virtual void handle(syscall_frame *f) = 0;
};

extern Syscall *syscall_table[static_cast<unsigned>(SyscallNum::MAX_SYSCALL)];
