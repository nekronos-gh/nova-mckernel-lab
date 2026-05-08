#include "scheduler.h"
#include "stdio.h"

Scheduler Scheduler::sched;

void Scheduler::enqueue(Ec *ec) {
    if (count >= MAX_ECS) {
        panic("Scheduler: too many ECs\n");
    }
    slots[tail] = ec;
    tail = (tail + 1) % MAX_ECS;
    count++;
}

Ec *Scheduler::yield() {
    if (count == 0) {
        return nullptr;
    }
    if (count == 1) {
        return slots[head];
    }

    // Move current to the back
    Ec *current = slots[head];
    head = (head + 1) % MAX_ECS;

    slots[tail] = current;
    tail = (tail + 1) % MAX_ECS;

    // The new head is the next to run
    return slots[head];
}
