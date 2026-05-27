#ifndef KSCHED_H
#define KSCHED_H

#include "kthread.h"

#ifdef __cplusplus
extern "C" {
#endif

tcb *ksched_get();
void ksched_put(tcb *thread);

#ifdef __cplusplus
}
#endif

#endif // KSCHED_H
