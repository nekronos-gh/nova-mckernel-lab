#include "syscall.h"
#include "ec.h"
#include "kalloc.h"
#include "memory.h"
#include "ptab.h"
#include "scheduler.h"
#include "stdio.h"

class SyscallMmap : public Syscall {
  public:
    void handle(syscall_frame *f) override {
        // Print number of arguments
        if (f->argc < 2)
            return;
        // Argv[0] = Number of pages
        unsigned n_pages = f->argv[0];
        // Argv[1] = Start address
        mword virt = f->argv[1];
        for (unsigned i = 0; i < n_pages; ++i) {
            // Allocate a physical page and map to viratual memory
            void *page = Kalloc::allocator.alloc_page(1, Kalloc::FILL_0);
            mword phys = Kalloc::virt2phys(page);
            Ptab::insert_mapping(virt, phys, 7);
            virt += PAGE_SIZE;
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
        if (f->argc < 2)
            return;

        const char *fmt = reinterpret_cast<const char *>(f->argv[0]);

        // Variadic ABI layout
        // By convention, the argv[1] contains an address list of arguments
        serial.vprintf(fmt, reinterpret_cast<va_list>(f->argv[1]));
    }
};

class SyscallClone : public Syscall {
  public:
    void handle(syscall_frame *f) override {
        if (f->argc < 3)
            return;
        syscall_clone *clone_frame = static_cast<syscall_clone *>(f);
        Ec *user_ec = new Ec(clone_frame->eip(), clone_frame->esp(),
                             clone_frame->priority());
        Scheduler::sched.schedule(user_ec);
        Ec::current->sys_regs()->eax = reinterpret_cast<mword>(user_ec);
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
        Ec *target =
            (f->argc == 0) ? Ec::current : reinterpret_cast<Ec *>(f->argv[0]);
        Ec *next = Scheduler::sched.block(target);
        if (next) {
            next->make_current();
        }
    }
};

class SyscallUnblock : public Syscall {
  public:
    void handle(syscall_frame *f) override {
        if (f->argc < 1) {
            Scheduler::sched.unblock_all();
        } else {
            Ec *target = reinterpret_cast<Ec *>(f->argv[0]);
            Scheduler::sched.unblock(target);
        }
    }
};

static SyscallMmap sys_mmap_h;
static SyscallDump sys_dump_h;
static SyscallPrint sys_print_h;
static SyscallClone sys_clone_h;
static SyscallYield sys_yield_h;
static SyscallBlock sys_block_h;
static SyscallUnblock sys_unblock_h;

Syscall *syscall_table[static_cast<unsigned>(SyscallNum::MAX_SYSCALL)] = {
    [static_cast<unsigned>(SyscallNum::SYS_MMAP)] = &sys_mmap_h,
    [static_cast<unsigned>(SyscallNum::SYS_DUMP)] = &sys_dump_h,
    [static_cast<unsigned>(SyscallNum::SYS_PRINT)] = &sys_print_h,
    [static_cast<unsigned>(SyscallNum::SYS_CLONE)] = &sys_clone_h,
    [static_cast<unsigned>(SyscallNum::SYS_YIELD)] = &sys_yield_h,
    [static_cast<unsigned>(SyscallNum::SYS_BLOCK)] = &sys_block_h,
    [static_cast<unsigned>(SyscallNum::SYS_UNBLOCK)] = &sys_unblock_h};
