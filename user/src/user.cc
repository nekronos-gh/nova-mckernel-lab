#define NORETURN __attribute__((noreturn))
#define EXTERN_C extern "C"

unsigned syscall1(unsigned w0) {
    asm volatile("   mov %%esp, %%ecx    ;"
                 "   mov $1f, %%edx      ;"
                 "   sysenter            ;"
                 "1:                     ;"
                 : "+a"(w0)
                 :
                 : "ecx", "edx");
    return w0;
}

unsigned syscall2(unsigned w0, unsigned w1) {
    asm volatile("   mov %%esp, %%ecx    ;"
                 "   mov $1f, %%edx      ;"
                 "   sysenter            ;"
                 "1:                     ;"
                 : "+a"(w0)
                 : "S"(w1)
                 : "ecx", "edx");
    return w0;
}

// example syscall for yielding
unsigned sys_yield() {
    return syscall1(0xdeadbeaf /* TODO use correct syscall number*/);
}

unsigned sys_print(const char *str) { return syscall2(1, (unsigned)str); }

unsigned clone(void (*f)()) { return syscall2(2, (unsigned)f); }

EXTERN_C void th1() {
    // sys_print("Hello from thread 1!\n");
    while (1)
        ;
}

EXTERN_C NORETURN void main_func() {
    sys_print("Calling th1!\n");
    clone(&th1);
    sys_print("Done!\n");
    while (1)
        ;
}
