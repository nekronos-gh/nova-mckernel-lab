#pragma once

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

struct syscall_frame {
    SyscallNum num;
    unsigned argc;
    unsigned argv[4]; // Max four arguments
};

// Raw syscall with frame for arguments
__attribute__((always_inline)) static inline unsigned
syscall_raw(struct syscall_frame *f) {

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
#define VA_NARGS_IMPL(_0, _1, _2, _3, _4, N, ...) N
// Extracts number of args passed to __VA_ARGS__
#define VA_NARGS(...) VA_NARGS_IMPL(, ##__VA_ARGS__, 4, 3, 2, 1, 0)

// Builds the actual syscall frame structure explicitly
// This is the final object passed to the kernel by reference
#define SYSCALL_BUILD_FRAME(num, argc, a0, a1, a2, a3)                         \
    syscall_frame {                                                            \
        (num), (argc), { (a0), (a1), (a2), (a3) }                              \
    }

// Helper layer to force argument expansion order
#define _SYSCALL_EXPAND(num, argc, a0, a1, a2, a3, ...)                        \
    SYSCALL_BUILD_FRAME(num, argc, a0, a1, a2, a3)

// Public syscall macro
#define syscall(num, ...)                                                      \
    ({                                                                         \
        syscall_frame f = _SYSCALL_EXPAND(num, VA_NARGS(__VA_ARGS__),          \
                                          ##__VA_ARGS__, 0, 0, 0, 0);          \
        syscall_raw(&f);                                                       \
    })
