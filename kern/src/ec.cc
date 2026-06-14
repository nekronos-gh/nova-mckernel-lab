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

#include "ec.h"
#include "bits.h"
#include "cpu.h"
#include "elf.h"
#include "multiboot.h"
#include "pd.h"
#include "ptab.h"
#include "scheduler.h"

Ec *Ec::current = 0;

// solely used for root_invoke()
Ec::Ec(void (*f)(), mword mbi, Pd *p)
    : cont(f), priority(0), blocked(false), ipc_val(0), pd(p) {
    regs.eax = mbi;
    regs.cs = SEL_USER_CODE;
    regs.ds = SEL_USER_DATA;
    regs.es = SEL_USER_DATA;
    regs.ss = SEL_USER_DATA;
    regs.efl = 0x200; // IF = 1
}

// only used by syscall create thread (EC+SC)
Ec::Ec(mword eip, mword esp, unsigned prio, Pd *p)
    : cont(ret_user_iret), priority(prio), blocked(false), ipc_val(0), pd(p) {
    regs.cs = SEL_USER_CODE;
    regs.ds = SEL_USER_DATA;
    regs.es = SEL_USER_DATA;
    regs.ss = SEL_USER_DATA;
    regs.efl = 0x200; // IF = 1
    regs.eip = eip;
    regs.esp = esp;
}

void Ec::ret_user_sysexit() {
    asm volatile("lea %0, %%esp;"
                 "popa;"
                 "sti;"
                 "sysexit"
                 :
                 : "m"(current->regs)
                 : "memory");

    UNREACHED;
}

void Ec::ret_user_iret() {

    // Dump all registers for debugging purposes
    asm volatile("lea %0, %%esp;"
                 "popa;"
                 "pop %%gs;"
                 "pop %%fs;"
                 "pop %%es;"
                 "pop %%ds;"
                 "add $8, %%esp;"
                 "iret"
                 :
                 : "m"(current->regs)
                 : "memory");

    UNREACHED;
}

void Ec::root_invoke() {
    // find multi boot info
    Multiboot *mbi = static_cast<Multiboot *>(Ptab::remap(current->regs.eax));

    if (!(mbi->flags & 8) || (mbi->mods_count != 1))
        panic("[kern::Ec::root_invoke] exactly ONE multi boot module is "
              "required.\n");

    // load module desciptor
    Multiboot_module mod =
        *static_cast<Multiboot_module *>(Ptab::remap(mbi->mods_addr));

    printf("[kern::root_invoke]\t load module from %x - %x (%u bytes) : ",
           mod.mod_start, mod.mod_end, mod.mod_end - mod.mod_start);
    char *cmd = static_cast<char *>(Ptab::remap(mod.cmdline));
    printf("[kern::root_invoke]\t %s\n", cmd);

    // remap elf header
    Eh *e = static_cast<Eh *>(Ptab::remap(mod.mod_start));
    if (e->ei_magic != 0x464c457f || e->ei_data != 1 || e->type != 2)
        panic("[kern::Ec::root_invoke] No ELF\n");

    unsigned count = e->ph_count;
    current->regs.eip = e->entry;

    // remap program headers
    Ph *p = static_cast<Ph *>(Ptab::remap(mod.mod_start + e->ph_offset));

    for (; count--; p++) {

        if (p->type == Ph::PT_LOAD) {

            unsigned attr = p->flags & Ph::PF_W ? 7 : 5;

            if (p->f_size != p->m_size ||
                p->v_addr % PAGE_SIZE != p->f_offs % PAGE_SIZE)
                panic("[kern::Ec::root_invoke] Bad ELF\n");

            mword phys = align_dn(p->f_offs + mod.mod_start, PAGE_SIZE);
            mword virt = align_dn(p->v_addr, PAGE_SIZE);
            mword size = align_up(p->f_size, PAGE_SIZE);

            while (size) {
                Ptab::insert_mapping(virt, phys, attr);
                virt += PAGE_SIZE;
                phys += PAGE_SIZE;
                size -= PAGE_SIZE;
            }
        }
    }

    // Push the root process into our stack
    Scheduler::sched.schedule(current);
    // Set a normal return for when yielded back
    current->cont = ret_user_iret;

    ret_user_iret();

    FAIL;
}

void Ec::handle_tss() { panic("[kern::Ec::handle_tss] Task gate invoked\n"); }

void Ec::handle_syscall(syscall_frame *f) {
    // By convention
    // EAX = pointer to the syscall frame
    // ECX = pointer to user stack
    // EDX = return address to the user code
    // Last pushed element references syscall arguments

    SyscallNum num = f->num;

    if (num < SyscallNum::MAX_SYSCALL &&
        syscall_table[static_cast<unsigned>(num)]) {
        syscall_table[static_cast<unsigned>(num)]->handle(f);
    } else {
        printf("[kern::handle_syscall]\t unknown syscall %d\n",
               static_cast<int>(num));
    }

    ret_user_sysexit();

    UNREACHED;
}

bool Ec::handle_exc_ts(Exc_regs *r) {
    if (r->user())
        return false;

    // SYSENTER with EFLAGS.NT=1 and IRET faulted
    r->efl &= ~0x4000; // nested task eflag

    return true;
}

void Ec::handle_exc(Exc_regs *r) {
    if (r->vec == Cpu::EXC_TS && handle_exc_ts(r))
        return;

    if (r->vec == Cpu::EXC_GP)
        panic("[kern::Ec::handle_exc] %s GP (EIP=%#lx CR2=%#lx)\n",
              r->eip < LINK_ADDR ? "User" : "Kernel", r->eip, r->cr2);

    if (r->vec == Cpu::EXC_PF) {
        bool is_main_stack_guard = (r->cr2 >= USER_MAIN_STACK_START &&
                                    r->cr2 < USER_MAIN_STACK_START + PAGE_SIZE);
        bool is_heap_stack_guard =
            (r->cr2 >= USER_HEAP_START && r->cr2 < USER_HEAP_END &&
             ((r->cr2 - USER_HEAP_START) % USER_STACK_SIZE < PAGE_SIZE));

        if (is_main_stack_guard || is_heap_stack_guard)
            panic("[kern::Ec::handle_exc] User Stack Overflow (EIP=%#lx "
                  "CR2=%#lx)\n",
                  r->eip, r->cr2);

        panic("[kern::Ec::handle_exc] %s PF (EIP=%#lx CR2=%#lx)\n",
              r->eip < LINK_ADDR ? "User" : "Kernel", r->eip, r->cr2);
    }

    panic("[kern::Ec::handle_exc] %s EXC %#lx (EIP=%#lx CR2=%#lx)\n",
          r->eip < LINK_ADDR ? "User" : "Kernel", r->vec, r->eip, r->cr2);

    UNREACHED;
}
