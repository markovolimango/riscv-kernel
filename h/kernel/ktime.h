#ifndef KTIME_H
#define KTIME_H

#include "../../lib/hw.h"

#ifdef __cplusplus
extern "C" {
#endif

void ktime_tick();

int ktime_sleep(time_t ticks);

#ifdef __cplusplus
}
#endif

#endif // KTIME_H
