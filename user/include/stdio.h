#pragma once
#include "syscall.h"

template <typename... Args>
static unsigned printf(char const *fmt, Args... args) {
    mword _args[] = {reinterpret_cast<mword>(args)..., 0};
    return syscall(SYS_PRINT, reinterpret_cast<mword>(fmt),
                   reinterpret_cast<mword>(_args));
}
