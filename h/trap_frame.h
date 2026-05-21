#ifndef TRAP_FRAME_H
#define TRAP_FRAME_H

#include "../lib/hw.h"

typedef struct {
    uint64 x[32];
} trap_frame;

#endif // TRAP_FRAME_H