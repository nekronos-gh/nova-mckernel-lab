#pragma once

class Ec;

// Simple round robin scheduler
class Scheduler {
  private:
    // Max user thread per process
    static constexpr unsigned MAX_ECS = 16;
    Ec *slots[MAX_ECS];
    unsigned head = 0;
    unsigned tail = 0;
    unsigned count = 0;

  public:
    static Scheduler sched;

    Scheduler() : head(0), tail(0), count(0) {}

    void enqueue(Ec *ec);

    Ec *yield();
};
