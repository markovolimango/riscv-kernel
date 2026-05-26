#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "kthread.h"

#ifdef __cplusplus
extern "C" {
#endif

tcb *scheduler_get();
void scheduler_put(tcb *thread);

#ifdef __cplusplus
}
#endif

#endif // SCHEDULER_H