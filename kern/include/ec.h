/*
 * Execution Context
 *
 * Copyright (C) 2009-2011 Udo Steinberg <udo@hypervisor.org>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * This file is part of the NOVA microhypervisor.
 *
 * NOVA is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * NOVA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License version 2 for more details.
 */

#pragma once

#include "compiler.h"
#include "kalloc.h"
#include "memory.h"
#include "regs.h"
#include "stdio.h"
#include "string.h"
#include "syscall.h"
#include "tss.h"

class Ec {
  private:
    void (*cont)();
    Exc_regs regs;

    REGPARM(1)
    static void handle_exc(Exc_regs *) asm("exc_handler");

    NORETURN
    static void handle_tss() asm("tss_handler");

    static bool handle_exc_ts(Exc_regs *);

  public:
    ALWAYS_INLINE
    inline Sys_regs *sys_regs() { return &regs; }

    ALWAYS_INLINE
    inline Exc_regs *exc_regs() { return &regs; }
    static Ec *current;

    Ec(void (*)(), mword = 0);
    Ec(mword, mword);

    ALWAYS_INLINE NORETURN inline void make_current() {
        current = this;

        Tss::run.sp0 = reinterpret_cast<mword>(exc_regs() + 1);

        asm volatile("mov %0, %%esp;"
                     "jmp *%1"
                     :
                     : "g"(KSTCK_ADDR + PAGE_SIZE), "rm"(cont)
                     : "memory");
        UNREACHED;
    }

    HOT NORETURN static void ret_user_sysexit();

    NORETURN
    static void ret_user_iret() asm("ret_user_iret");

    NORETURN
    static void root_invoke();

    HOT NORETURN REGPARM(1) static void handle_syscall(
        struct syscall_frame *) asm("syscall_handler");

    ALWAYS_INLINE
    static inline void *operator new(size_t) {
        return Kalloc::allocator.alloc(sizeof(Ec));
    }

    ALWAYS_INLINE
    static inline void operator delete(void *) { /* nop */ }
};

struct UserEcStack {
  public:
    mword *slots[PAGE_SIZE / sizeof(mword *)];
    unsigned count = 0;

    void push(mword *addr) {
        if (count < (PAGE_SIZE / sizeof(mword *))) {
            slots[count++] = addr;
        }
    }

    mword *current() {
        if (!count) {
            return 0;
        }
        return slots[0];
    }

    mword *yield() {
        if (!count) {
            return 0;
        }
        if (count < 2) {
            return slots[0];
        }

        mword *first = slots[0];
        for (unsigned i = 1; i < count; ++i) {
            slots[i - 1] = slots[i];
        }
        slots[count - 1] = first;
        return slots[0];
    }
};
