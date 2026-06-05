#ifndef KSCHED_H
#define KSCHED_H

#include "kthread.h"

#ifdef __cplusplus
extern "C" {
#endif

void ksched_init();
void ksched_put(thread *t);
void ksched_switch();

#ifdef __cplusplus
}
#endif

#endif // KSCHED_H
