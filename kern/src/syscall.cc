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
        syscall_create_ec *frame = static_cast<syscall_create_ec *>(f);

        Pd *target_pd = Ec::current->pd;

        // Cloned EC inherits the same protection domain as the caller
        Ec *user_ec =
            new Ec(frame->eip(), frame->esp(), frame->priority(), target_pd);

        int capability = target_pd->set_cap(frame->capability(), user_ec);
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

class SyscallGetCapSlot : public Syscall {
  public:
    void handle(syscall_frame *) override {
        for (unsigned i = 0; i < MAX_CAPS; i++) {
            if (Ec::current->pd->get_cap(i) == Ec::current) {
                Ec::current->sys_regs()->eax = i;
                return;
            }
        }
    }
};

class SyscallCreatePD : public Syscall {
  public:
    void handle(syscall_frame *) override {
        Pd *caller_pd = Ec::current->pd;

        // Allocate child PD with caller as parent
        Pd *child = new Pd(caller_pd);
        if (!child) {
            Ec::current->sys_regs()->eax = static_cast<mword>(-1);
            return;
        }

        int idx = caller_pd->add_child(child);
        if (idx < 0) {
            // FIX: Memory leak but no delete op exists
            Ec::current->sys_regs()->eax = static_cast<mword>(-1);
            return;
        }

        // Return the child-index so user space can refer to this PD later
        Ec::current->sys_regs()->eax = static_cast<mword>(idx);
    }
};

class SyscallDelegateCap : public Syscall {
  public:
    void handle(syscall_frame *f) override {
        syscall_delegate_cap *frame = static_cast<syscall_delegate_cap *>(f);

        if (frame->argc < 3) {
            Ec::current->sys_regs()->eax = static_cast<mword>(-1);
            return;
        }

        Pd *caller_pd = Ec::current->pd;

        if (frame->child_idx >= MAX_CAPS ||
            caller_pd->children[frame->child_idx] == nullptr) {
            Ec::current->sys_regs()->eax = static_cast<mword>(-1);
            return;
        }

        Pd *child_pd = caller_pd->children[frame->child_idx];

        Ec *ec = caller_pd->get_cap(frame->src_slot);
        if (!ec) {
            Ec::current->sys_regs()->eax = static_cast<mword>(-1);
            return;
        }

        int result = child_pd->set_cap(frame->dst_slot, ec);
        if (result < 0) {
            Ec::current->sys_regs()->eax = static_cast<mword>(-1);
            return;
        }

        Ec::current->sys_regs()->eax = 0;
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
static SyscallGetCapSlot sys_get_cap_slot_h;
static SyscallCreatePD sys_create_pd_h;
static SyscallDelegateCap sys_delegate_cap_h;

Syscall *syscall_table[static_cast<unsigned>(SyscallNum::MAX_SYSCALL)] = {
    [static_cast<unsigned>(SyscallNum::SYS_MMAP)] = &sys_mmap_h,
    [static_cast<unsigned>(SyscallNum::SYS_DUMP)] = &sys_dump_h,
    [static_cast<unsigned>(SyscallNum::SYS_PRINT)] = &sys_print_h,
    [static_cast<unsigned>(SyscallNum::SYS_CREATE_EC)] = &sys_create_ec_h,
    [static_cast<unsigned>(SyscallNum::SYS_YIELD)] = &sys_yield_h,
    [static_cast<unsigned>(SyscallNum::SYS_BLOCK)] = &sys_block_h,
    [static_cast<unsigned>(SyscallNum::SYS_UNBLOCK)] = &sys_unblock_h,
    [static_cast<unsigned>(SyscallNum::SYS_CHECK_CAP)] = &sys_check_cap_h,
    [static_cast<unsigned>(SyscallNum::SYS_ADD_CAP)] = &sys_add_cap_h,
    [static_cast<unsigned>(SyscallNum::SYS_GET_CAP_SLOT)] = &sys_get_cap_slot_h,
    [static_cast<unsigned>(SyscallNum::SYS_CREATE_PD)] = &sys_create_pd_h,
    [static_cast<unsigned>(SyscallNum::SYS_DELEGATE_CAP)] = &sys_delegate_cap_h,

};
