#include "scheduler.h"
#include "stdio.h"

Scheduler Scheduler::sched;

void Scheduler::schedule(Ec *ec) {
    Queue &q = queues[ec->priority];
    if (q.count >= MAX_ECS)
        panic("Scheduler: queue full at priority %u\n", ec->priority);
    q.slots[q.tail] = ec;
    q.blocked[q.tail] = 0; // Not blocked by default
    q.tail = (q.tail + 1) % MAX_ECS;
    q.count++;
    // If no EC is currently running, yield to schedule this new EC immediately
    if (!current_ec) {
        yield();
    }
}

Ec *Scheduler::yield() {
    // Re-enqueue current EC at the tail of its priority queue, marked runnable.
    if (current_ec) {
        Queue &q = queues[current_ec->priority];
        q.slots[q.tail] = current_ec;
        q.blocked[q.tail] = 0; // Clear any stale blocked flag in this slot
        q.tail = (q.tail + 1) % MAX_ECS;
        q.count++;
        current_ec = nullptr;
    }

    // Find the first non-blocked EC, highest priority first
    for (unsigned prio = 0; prio < NUM_PRIORITIES; prio++) {

        Queue &q = queues[prio];
        if (q.count == 0)
            continue;

        // Find the first non-blocked EC in this priority queue
        for (unsigned i = 0; i < q.count; i++) {
            Ec *ec = q.slots[q.head];
            uint8 blocked = q.blocked[q.head];
            q.head = (q.head + 1) % MAX_ECS;

            if (!blocked) {
                q.count--;
                current_ec = ec;
                return ec;
            }

            // Rotate it to the tail so it stays in the queue
            q.slots[q.tail] = ec;
            q.blocked[q.tail] = 1;
            q.tail = (q.tail + 1) % MAX_ECS;
        }
        // Try the next priority
    }

    // Nothing runnable
    return nullptr;
}

Ec *Scheduler::block(Ec *ec) {
    // If EC is currently running, mark it blocked and yield immediately
    if (ec == current_ec) {
        Queue &q = queues[ec->priority];
        q.slots[q.tail] = ec;
        q.blocked[q.tail] = 1; // Enqueue as blocked before yielding
        q.tail = (q.tail + 1) % MAX_ECS;
        q.count++;
        current_ec = nullptr;
        return yield();
    }

    // Mark the EC as blocked in its queue
    Queue &q = queues[ec->priority];
    for (unsigned i = 0; i < q.count; i++) {
        unsigned idx = (q.head + i) % MAX_ECS;
        if (q.slots[idx] == ec) {
            q.blocked[idx] = 1;
            break;
        }
    }
    return nullptr;
}

void Scheduler::unblock(Ec *ec) {
    // Find the EC in its queue and clear the blocked flag
    Queue &q = queues[ec->priority];
    for (unsigned i = 0; i < q.count; i++) {
        unsigned idx = (q.head + i) % MAX_ECS;
        if (q.slots[idx] == ec) {
            q.blocked[idx] = 0;
            return;
        }
    }
}

void Scheduler::unblock_all() {
    for (unsigned prio = 0; prio < NUM_PRIORITIES; prio++) {
        Queue &q = queues[prio];
        for (unsigned i = 0; i < q.count; i++) {
            unsigned idx = (q.head + i) % MAX_ECS;
            q.blocked[idx] = 0;
        }
    }
}
