#include "../../h/kernel/ktime.h"
#include "../../h/kernel/kthread.h"

void ktime_tick() {
    running_thread->time_slice -= 1;
    if (running_thread->time_slice == 0) {
        running_thread->time_slice = DEFAULT_TIME_SLICE;
        kthread_dispatch();
    }
}
