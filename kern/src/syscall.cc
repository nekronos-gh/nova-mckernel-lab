#include "syscall.h"
#include "ec.h"
#include "kalloc.h"
#include "memory.h"
#include "pd.h"
#include "ptab.h"
#include "scheduler.h"
#include "stdio.h"

class SyscallMmap : public Syscall {
  public:
    void handle(syscall_frame *f) override {
        syscall_mmap *frame = static_cast<syscall_mmap *>(f);

        for (unsigned i = 0; i < frame->n_pages; ++i) {
            // Allocate a physical page and map to virtual memory
            void *page = Kalloc::allocator.alloc_page(1, Kalloc::FILL_0);
            mword phys = Kalloc::virt2phys(page);
            Ptab::insert_mapping(frame->virt_addr, phys, 7);
            frame->virt_addr += PAGE_SIZE;
        }
    }
};

class SyscallDump : public Syscall {
  public:
    void handle(syscall_frame *) override {
        printf("EC:%p SYS_DUMP : %#lx, %#lx\n", Ec::current,
               Ec::current->sys_regs()->esi, Ec::current->sys_regs()->edi);
    }
};

class SyscallPrint : public Syscall {
  public:
    void handle(syscall_frame *f) override {
        syscall_print *frame = static_cast<syscall_print *>(f);
        serial.vprintf(frame->fmt, frame->args);
    }
};

class SyscallCreateEC : public Syscall {
  public:
    void handle(syscall_frame *f) override {
        if (f->argc < 3)
            return;
        syscall_create_ec *create_ec_frame =
            static_cast<syscall_create_ec *>(f);
        // Cloned EC inherits the same protection domain as the caller
        Ec *user_ec = new Ec(create_ec_frame->eip(), create_ec_frame->esp(),
                             create_ec_frame->priority(), Ec::current->pd);

        int capability =
            Ec::current->pd->set_cap(create_ec_frame->capability(), user_ec);
        if (capability < 0) {
            // FIX: We do not have a delete operator
            // This is a memory leak
            return;
        }

        Scheduler::sched.schedule(user_ec);
        Ec::current->sys_regs()->eax = static_cast<mword>(capability);
    }
};

class SyscallYield : public Syscall {
  public:
    void handle(syscall_frame *) override {
        Ec *next = Scheduler::sched.yield();
        if (!next)
            return;
        next->make_current();
    }
};

class SyscallBlock : public Syscall {
  public:
    void handle(syscall_frame *f) override {
        syscall_block *frame = static_cast<syscall_block *>(f);

        Ec *target = (frame->argc == 0)
                         ? Ec::current
                         : Ec::current->pd->get_cap(frame->capability);

        if (!target)
            return;

        Ec *next = Scheduler::sched.block(target);
        if (next) {
            next->make_current();
        }
    }
};

class SyscallUnblock : public Syscall {
  public:
    void handle(syscall_frame *f) override {
        syscall_unblock *frame = static_cast<syscall_unblock *>(f);

        if (frame->argc == 0) {
            Scheduler::sched.unblock_all();
        } else {
            Ec *target = Ec::current->pd->get_cap(frame->capability);
            if (!target)
                return;
            Scheduler::sched.unblock(target);
        }
    }
};

class SyscallCheckCap : public Syscall {
  public:
    void handle(syscall_frame *f) override {
        syscall_check_cap *frame = static_cast<syscall_check_cap *>(f);
        unsigned slot = frame->capability;
        if (slot >= MAX_CAPS) {
            Ec::current->sys_regs()->eax = 0;
            return;
        }
        Ec::current->sys_regs()->eax = Ec::current->capabilities[slot];
    }
};

class SyscallAddCap : public Syscall {
  public:
    void handle(syscall_frame *f) override {
        syscall_add_cap *frame = static_cast<syscall_add_cap *>(f);
        unsigned slot = frame->capability;
        if (slot >= MAX_CAPS)
            return;
        Ec::current->capabilities[slot] = 1;
    }
};

static SyscallMmap sys_mmap_h;
static SyscallDump sys_dump_h;
static SyscallPrint sys_print_h;
static SyscallCreateEC sys_create_ec_h;
static SyscallYield sys_yield_h;
static SyscallBlock sys_block_h;
static SyscallUnblock sys_unblock_h;
static SyscallCheckCap sys_check_cap_h;
static SyscallAddCap sys_add_cap_h;

Syscall *syscall_table[static_cast<unsigned>(SyscallNum::MAX_SYSCALL)] = {
    [static_cast<unsigned>(SyscallNum::SYS_MMAP)] = &sys_mmap_h,
    [static_cast<unsigned>(SyscallNum::SYS_DUMP)] = &sys_dump_h,
    [static_cast<unsigned>(SyscallNum::SYS_PRINT)] = &sys_print_h,
    [static_cast<unsigned>(SyscallNum::SYS_CREATE_EC)] = &sys_create_ec_h,
    [static_cast<unsigned>(SyscallNum::SYS_YIELD)] = &sys_yield_h,
    [static_cast<unsigned>(SyscallNum::SYS_BLOCK)] = &sys_block_h,
    [static_cast<unsigned>(SyscallNum::SYS_UNBLOCK)] = &sys_unblock_h,
    [static_cast<unsigned>(SyscallNum::SYS_CHECK_CAP)] = &sys_check_cap_h,
    [static_cast<unsigned>(SyscallNum::SYS_ADD_CAP)] = &sys_add_cap_h};
