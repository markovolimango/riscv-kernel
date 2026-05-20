#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "_thread.hpp"

class Scheduler {
  public:
    static void put(_thread *thread);
    static _thread *get();

  private:
    static _thread *head;
    static _thread *tail;
};

#endif