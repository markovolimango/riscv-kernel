#ifndef ERRNO_HPP
#define ERRNO_HPP

// same as POSIX errno values
#define EOK 0         // Success
#define ENOSYS 38     // Function not implemented (invalid syscall code)
#define ENOMEM 12     // Out of memory (mem_alloc failed)
#define EINVAL 22     // Invalid argument (bad handle, null ptr, bad size, etc.)
#define EBADF 9       // Bad file descriptor — for bad sem_t / thread_t handles
#define EDEADLK 35    // Deadlock would occur (optional, sem edge cases)
#define ECANCELED 125 // Operation canceled (sem_close while waiting)
#define ESRCH 3       // No such process/thread (thread already exited)
#define ETIME 62      // Timer expired (optional, sleep edge cases)

#endif // ERRNO_HPP