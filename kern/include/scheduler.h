#pragma once

class Ec;

class Scheduler {
    static constexpr unsigned NUM_PRIORITIES = 2;
    static constexpr unsigned MAX_ECS = 8; // per-priority queue

    struct Queue {
        Ec *slots[MAX_ECS];
        unsigned head = 0, tail = 0, count = 0;
        Queue() : head(0), tail(0), count(0) {
            for (unsigned i = 0; i < MAX_ECS; i++)
                slots[i] = 0;
        }
    };

    Queue queues[NUM_PRIORITIES];

  public:
    static Scheduler sched;

    void enqueue(Ec *ec);
    Ec *yield();
};
