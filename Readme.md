# Nova Microhypervisor

Implementation of a minimal microkernel / microhypervisor as part of the _Microkernel-Based Systems_ coursework.

This project incrementally builds core OS concepts including execution contexts, scheduling, system calls, capabilities, and inter-process communication (IPC).

---

## Overview

The goal is to implement a small microkernel that supports:

- Execution contexts (threads)
- Cooperative and priority-based scheduling
- System calls
- Blocking / unblocking
- Capability-based access control
- Synchronous IPC

---

## Project Structure

- `ec.cc` – Execution context logic and syscall handling
- `scheduler/` – Scheduling and ready/blocked queues
- `syscalls/` – System call implementations
- `mkc/user/` – User-level test programs
- `root_invoke` – Kernel entry that loads initial user ELF

---

## Exercises

### Exercise II – Core Kernel (Scheduling & Syscalls)

#### Task 1: Simple Scheduler (40%)

- Implement a ready list for execution contexts (ECs)
- Support:
  - Enqueue at tail
  - Dequeue from head
- Implement:
  - `create_ec` system call (create thread in same address space)
  - `yield` system call (cooperative scheduling)
- Ensure correct initialization of kernel and user ECs
- Add debug output for scheduling behavior

---

#### Task 2: Priority Scheduling (30%)

- Extend ready list to support multiple priority levels
- Modify `create_ec` to accept priority
- Scheduling rules:
  - Highest priority ECuns first
  - Same priority → round-robin scheduling
- Create test case:
  - 4 threads, 2 priorities

---

#### Task 3: Blocking (30%)

- Add blocked EC list
- Implement:
  - `block` system call (moves EC to blocked list)
  - `unblock_all` system call (moves all blocked ECs back to ready list)
- Ensure correct scheduler behavior after blocking/unblocking

---

### Exercise III – Advanced Kernel Features

#### Task 4: Capabilities (30%)

- Implement capability system for ECs
- Extend `create_ec` with capability slot assignment
- Maintain kernel data structure mapping slots to kernel objects

---

#### Task 5: IPC (50%)

- Implement synchronous IPC system call
- Supports single register value transfer
- Behavior:
  - Sender/receiver block until counterpart is ready
- Ensure correct interaction with scheduler

---

#### Task 6: IPC with Priorities (20%)

- Extend IPC to work with priority scheduling
- Handle priority inversion scenarios
- Provide design + implementation strategy

---

## System Calls

- `sys_dump` – Debug output syscall
- `sys_yield` – Yield CPU to next ready EC
- `sys_create_ec` – Create new execution context
- `sys_block` – Block current EC
- `sys_unblock_all` – Unblock all ECs
- `sys_ipc_send / sys_ipc_recv` – Synchronous message passing

---

## Notes

- ELF loading and multiboot handling are assumed already provided
- Focus is on kernel logic, not bootloader implementation
- Debug output is essential for validation

---

## License

For educational use only.
