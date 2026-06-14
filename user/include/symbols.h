extern char _heap_f[];
extern char _heap_e[];

#define PAGE_SIZE 0x1000
#define USER_STACK_PAGES 1
#define USER_STACK_SIZE ((USER_STACK_PAGES + 1) * PAGE_SIZE)

#define MAX_CAPS 8
extern unsigned _ipc_buffer[MAX_CAPS];
extern unsigned _ipc_buffer_status[MAX_CAPS];
