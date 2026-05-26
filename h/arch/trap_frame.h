#ifndef TRAP_FRAME_H
#define TRAP_FRAME_H

#include "../../lib/hw.h"

typedef struct {
    uint64 x[32];
    uint64 sepc;
    uint64 sstatus;
} trap_frame;

#endif // TRAP_FRAME_H
