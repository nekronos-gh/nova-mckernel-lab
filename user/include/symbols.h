extern char _heap_start[];
extern char _heap_end[];
extern char stack_top[];
extern char _stack_guard[];

#define PAGE_SIZE 0x1000
#define USER_STACK_PAGES 2048
#define USER_STACK_SIZE ((USER_STACK_PAGES + 1) * PAGE_SIZE)
#define HEAP_SIZE (USER_STACK_SIZE * 2028)
