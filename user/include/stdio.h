#pragma once

#include "syscall.h"

inline void printf(const char *str) {
    syscall(SYS_PRINT, 0xF00DBABE, 0xdeadbeef, 0xDEADD00D);
}
