#include "../../h/api/PeriodicThread.hpp"
#include "../../h/syscall_c.h"

void PeriodicThread::run() {
    while (!toTerminate) {
        periodicActivation();
        time_sleep(period);
    }
}
