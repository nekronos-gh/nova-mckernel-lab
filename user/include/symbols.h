extern char _heap_f[];
extern char _heap_e[];
extern unsigned _ipc_buffer;
extern unsigned _ipc_buffer_status;

#define PAGE_SIZE 0x1000
// TODO: increase to test real memory allocation
#define USER_STACK_PAGES 1
#define USER_STACK_SIZE ((USER_STACK_PAGES + 1) * PAGE_SIZE)

#define MAX_CAPS 8
