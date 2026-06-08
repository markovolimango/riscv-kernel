#ifndef PERIODIC_THREAD_HPP
#define PERIODIC_THREAD_HPP

#include "Thread.hpp"

class PeriodicThread : public Thread {
  public:
    void terminate();

  protected:
    PeriodicThread(time_t period);
    virtual void periodicActivation() {}
    void run() override;

  private:
    time_t period;
    bool toTerminate;
};

inline PeriodicThread::PeriodicThread(time_t period) : period(period), toTerminate(false) {}

inline void PeriodicThread::terminate() { toTerminate = true; }

inline void PeriodicThread::run() {
    while (!toTerminate) {
        periodicActivation();
        time_sleep(period);
    }
}

#endif
