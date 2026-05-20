#ifndef SYSCALL_CODE_HPP
#define SYSCALL_CODE_HPP

#include "../lib/hw.h"

enum class SyscallCode : uint64 {
    MEM_ALLOC = 0x00,
    MEM_FREE = 0x01,
    THREAD_CREATE = 0x11,
    THREAD_EXIT = 0x12,
    THREAD_DISPATCH = 0x13
};

#endif // SYSCALL_CODE_HPP