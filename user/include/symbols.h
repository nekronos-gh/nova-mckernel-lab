extern char _heap_f[];
extern char _heap_e[];

#define PAGE_SIZE 0x1000
// TODO: increase to test real memory allocation
#define USER_STACK_PAGES 1
#define USER_STACK_SIZE ((USER_STACK_PAGES + 1) * PAGE_SIZE)
