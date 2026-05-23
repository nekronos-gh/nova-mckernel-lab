#include "scheduler.h"
#include "ec.h"
#include "stdio.h"

Scheduler Scheduler::sched;

void Scheduler::enqueue(Ec *ec) {
    Queue &q = queues[ec->priority];
    if (q.count >= MAX_ECS)
        panic("Scheduler: queue full at priority %u\n", ec->priority);
    q.slots[q.tail] = ec;
    q.tail = (q.tail + 1) % MAX_ECS;
    q.count++;
}

Ec *Scheduler::yield() {
    // Rotate current EC to tail of its priority queue
    Queue &cur_q = queues[Ec::current->priority];
    if (cur_q.count > 1) {
        Ec *cur = cur_q.slots[cur_q.head];
        cur_q.head = (cur_q.head + 1) % MAX_ECS;
        cur_q.slots[cur_q.tail] = cur;
        cur_q.tail = (cur_q.tail + 1) % MAX_ECS;
    }

    // Return head of highest non-empty queue
    for (int p = static_cast<int>(NUM_PRIORITIES) - 1; p >= 0; p--) {
        if (queues[p].count > 0)
            return queues[p].slots[queues[p].head];
    }
    return 0;
}
