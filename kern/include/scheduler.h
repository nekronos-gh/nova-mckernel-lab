#pragma once

class Ec;

class Scheduler {
    static constexpr unsigned NUM_PRIORITIES = 2;
    static constexpr unsigned MAX_ECS = 8; // per-priority queue

    struct Queue {
        Ec *slots[MAX_ECS] = {}; // initialized to nullptr
        unsigned head = 0, tail = 0, count = 0;
    };

    Queue queues[NUM_PRIORITIES];

  public:
    static Scheduler sched;

    void enqueue(Ec *ec);
    Ec *yield();
};
