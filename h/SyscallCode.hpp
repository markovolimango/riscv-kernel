#ifndef SYSCALL_CODE_HPP
#define SYSCALL_CODE_HPP

#include "../lib/hw.h"

enum class SyscallCode : uint64 { MEM_ALLOC = 0x00, MEM_FREE = 0x01 };

#endif // SYSCALL_CODE_HPP