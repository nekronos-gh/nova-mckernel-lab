#pragma once

extern char _mempool_f[];
extern char _mempool_e[];

#define PAGE_SIZE 0x1000
#define USER_STACK_PAGES 1
#define USER_STACK_SIZE ((USER_STACK_PAGES + 1) * PAGE_SIZE)
