#include "syscall.h"
#include "ec.h"
#include "stdio.h"

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
        if (f->argc < 1)
            return;

        const char *fmt = reinterpret_cast<const char *>(f->argv[0]);

        // Reinterpret argv[1..] as a va_list by pointing directly into the args
        // array Variadic ABI layout
        // By convention, the argv[1] contains an address list of arguments
        serial.vprintf(fmt, reinterpret_cast<va_list>(f->argv[1]));
    }
};

class SyscallClone : public Syscall {
  public:
    void handle(syscall_frame *) override {}
};

static SyscallDump sys_dump_h;
static SyscallPrint sys_print_h;
static SyscallClone sys_clone_h;

Syscall *syscall_table[static_cast<unsigned>(SyscallNum::MAX_SYSCALL)] = {
    [static_cast<unsigned>(SyscallNum::SYS_DUMP)] = &sys_dump_h,
    [static_cast<unsigned>(SyscallNum::SYS_PRINT)] = &sys_print_h,
    [static_cast<unsigned>(SyscallNum::SYS_CLONE)] = &sys_clone_h,
};
