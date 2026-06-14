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
        printf("[kern::SyscallDump::handle]\t EC:%p SYS_DUMP : %#lx, %#lx\n",
               Ec::current, Ec::current->sys_regs()->esi,
               Ec::current->sys_regs()->edi);
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
        if (f->argc < 4)
            return;
        syscall_create_ec *frame = static_cast<syscall_create_ec *>(f);

        // Select correct EC
        if (frame->target_pd >= MAX_CAPS) {
            printf("[kern::SyscallCreateEC::handle]\t ERROR: invalid target_pd "
                   "%d\n",
                   frame->target_pd);
            Ec::current->sys_regs()->eax = static_cast<mword>(-1);
            return;
        }
        Pd *target_pd = Ec::current->pd->children[frame->target_pd];

        // Create clone EC on target PD
        Ec *user_ec =
            new Ec(frame->eip(), frame->esp(), frame->priority(), target_pd);
        int capability = target_pd->set_cap(frame->capability(), user_ec);

        if (capability < 0) {
            // FIX: We do not have a delete operator
            // This is a memory leak
            printf("[kern::SyscallCreateEC::handle] ERROR: could not add "
                   "capability");
            return;
        }

        // Add EC to current PD
        capability = Ec::current->pd->set_cap(frame->capability(), user_ec);

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

class SyscallIpcSend : public Syscall {
  public:
    void handle(syscall_frame *f) override {
        syscall_ipc_send *frame = static_cast<syscall_ipc_send *>(f);
        Ec *caller = Ec::current;

        Ec *target = caller->pd->get_cap(frame->target_cap);
        if (!target) {
            printf("[kern::SyscallIpcSend::handle]\t ERROR: could not send "
                   "message to %u\n",
                   frame->target_cap);
            caller->sys_regs()->eax = static_cast<mword>(-1);
            return;
        }

        // Fast path: receiver is blocked waiting
        if (target->pending_sender == target) {
            target->pending_sender = nullptr;
            target->sys_regs()->eax = frame->value;

            // Target will have the data when scheduled
            Scheduler::sched.unblock(target);
            Ec *next = Scheduler::sched.yield();
            if (next)
                next->make_current();
            UNREACHED;
        }

        // Slow path: receiver not ready
        caller->ipc_val = frame->value;

        // Priority inheritance to prevent priority inversion
        if (caller->priority < target->priority) {
            target->saved_priority = target->priority;
            Scheduler::sched.reprioritize(target, caller->priority);
        }

        // Register on target's sender list
        caller->pending_sender = target->pending_sender;
        target->pending_sender = caller;
        Ec *next = Scheduler::sched.block(caller);
        if (next)
            next->make_current();
        UNREACHED;
    }
};

class SyscallIpcRecv : public Syscall {
  public:
    void handle(syscall_frame *) override {
        Ec *caller = Ec::current;

        if (caller->pending_sender != nullptr &&
            caller->pending_sender != caller) {
            Ec *sender = caller->pending_sender;
            caller->pending_sender = sender->pending_sender;

            caller->sys_regs()->eax = sender->ipc_val;

            // Restore boosted priority if no more senders
            if (caller->pending_sender == nullptr &&
                caller->saved_priority != 0) {
                Scheduler::sched.reprioritize(caller, caller->saved_priority);
                caller->saved_priority = 0;
            }

            Scheduler::sched.unblock(sender);
            return;
        }

        // No sender yet, block
        if (caller->saved_priority != 0) {
            Scheduler::sched.reprioritize(caller, caller->saved_priority);
            caller->saved_priority = 0;
        }
        caller->pending_sender = caller;
        Ec *next = Scheduler::sched.block(caller);
        if (next)
            next->make_current();
        UNREACHED;
    }
};

static SyscallMmap sys_mmap_h;
static SyscallDump sys_dump_h;
static SyscallPrint sys_print_h;
static SyscallCreateEC sys_create_ec_h;
static SyscallYield sys_yield_h;
static SyscallBlock sys_block_h;
static SyscallUnblock sys_unblock_h;
static SyscallCreatePD sys_create_pd_h;
static SyscallDelegateCap sys_delegate_cap_h;
static SyscallIpcSend sys_ipc_send_h;
static SyscallIpcRecv sys_ipc_recv_h;

Syscall *syscall_table[static_cast<unsigned>(SyscallNum::MAX_SYSCALL)] = {
    [static_cast<unsigned>(SyscallNum::SYS_MMAP)] = &sys_mmap_h,
    [static_cast<unsigned>(SyscallNum::SYS_DUMP)] = &sys_dump_h,
    [static_cast<unsigned>(SyscallNum::SYS_PRINT)] = &sys_print_h,
    [static_cast<unsigned>(SyscallNum::SYS_CREATE_EC)] = &sys_create_ec_h,
    [static_cast<unsigned>(SyscallNum::SYS_YIELD)] = &sys_yield_h,
    [static_cast<unsigned>(SyscallNum::SYS_BLOCK)] = &sys_block_h,
    [static_cast<unsigned>(SyscallNum::SYS_UNBLOCK)] = &sys_unblock_h,
    [static_cast<unsigned>(SyscallNum::SYS_CREATE_PD)] = &sys_create_pd_h,
    [static_cast<unsigned>(SyscallNum::SYS_DELEGATE_CAP)] = &sys_delegate_cap_h,
    [static_cast<unsigned>(SyscallNum::SYS_IPC_SEND)] = &sys_ipc_send_h,
    [static_cast<unsigned>(SyscallNum::SYS_IPC_RECV)] = &sys_ipc_recv_h,

};
