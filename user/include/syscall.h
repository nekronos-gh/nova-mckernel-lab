#pragma once

#include "types.h"

enum SyscallNum : uint8 {
    SYS_DUMP = 0,
    SYS_PRINT = 1,
    SYS_CLONE = 2,
    MAX_SYSCALL
};

struct syscall_frame {
    enum SyscallNum num;
    unsigned argc;
    unsigned argv[3]; // Max three arguments
};

// Raw syscall with frame for arguments
static inline unsigned syscall_raw(struct syscall_frame *f) {

    unsigned res;

    asm volatile(
        "   mov  %%esp, %%ecx ;" // Store esp in ecx by convention
        "   mov  $1f, %%edx   ;" // Store return address in edx by convention
        "   sysenter          ;"
        "1:                   ;"
        : "=a"(res)
        : "a"(f) // eax will be the argument to REGPARM(1) syscall_handler()
        : "ecx", "edx", "memory");
    return res;
}

// Shifts arguments so N lands in correct position, discards the rest
#define VA_NARGS_IMPL(_1, _2, _3, N, ...) N
// Extracts number of args passed to __VA_ARGS__
#define VA_NARGS(...) VA_NARGS_IMPL(__VA_ARGS__, 3, 2, 1, 0)

// Builds the actual syscall frame structure explicitly
// This is the final object passed to the kernel by reference
#define SYSCALL_BUILD_FRAME(num, argc, a0, a1, a2)                             \
    syscall_frame {                                                            \
        (num), (argc), { (a0), (a1), (a2) }                                    \
    }

// Helper layer to force argument expansion order
#define _SYSCALL_EXPAND(num, argc, a0, a1, a2, ...)                            \
    SYSCALL_BUILD_FRAME(num, argc, a0, a1, a2)

// Public syscall macro
#define syscall(num, ...)                                                      \
    ({                                                                         \
        syscall_frame f = _SYSCALL_EXPAND(num, VA_NARGS(__VA_ARGS__),          \
                                          ##__VA_ARGS__, 0, 0, 0);             \
        syscall_raw(&f);                                                       \
    })
