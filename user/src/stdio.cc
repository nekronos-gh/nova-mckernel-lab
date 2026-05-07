#include "stdio.h"
#include "syscall.h"

void printf(const char *str) {
    syscall(SYS_PRINT, reinterpret_cast<mword>(str));
}
