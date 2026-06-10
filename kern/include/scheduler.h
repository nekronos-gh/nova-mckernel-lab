#pragma once
#include "ec.h"
#include "types.h"

class Ec;

class Scheduler {
    static constexpr unsigned NUM_PRIORITIES = 2;
    static constexpr unsigned MAX_ECS = 8; // per-priority queue

    struct Queue {
        Ec *slots[MAX_ECS] = {}; // initialized to nullptr
        unsigned head = 0, tail = 0, count = 0;
        uint8 blocked[MAX_ECS] = {}; // Map of blocked ECs in the queue
    };

    Queue queues[NUM_PRIORITIES];
    Ec *current_ec = nullptr;

  public:
    static Scheduler sched;

    void schedule(Ec *ec);
    Ec *yield();

    Ec *block(Ec *ec);
    void unblock(Ec *ec);
    void unblock_all();
};
