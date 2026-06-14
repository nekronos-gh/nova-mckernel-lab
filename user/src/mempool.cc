#include "mempool.h"
#include "stdio.h"
#include "symbols.h"
#include "syscall.h"

UserMemPool UserMemPool::heap(reinterpret_cast<mword>(&_mempool_f),
                              reinterpret_cast<mword>(&_mempool_e));

void *UserMemPool::alloc_stack() {
    if (begin + USER_STACK_SIZE > end) {
        printf("[user::UserMemPool::alloc_stack] ERROR: out of memory "
               "(begin=%#lx, end=%#lx)\n",
               begin, end);
        return nullptr;
    }

    // First page of a stack is a Stack Overflow Sentinel
    syscall(SYS_MMAP, USER_STACK_PAGES, begin + PAGE_SIZE);
    begin += USER_STACK_SIZE;

    // Return end address of the stack allocated
    return reinterpret_cast<void *>(begin);
}
