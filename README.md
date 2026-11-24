# riscv-kernel

A small preemptive multithreading kernel for 64-bit RISC-V, written in C with some C++ and RISC-V assembly. It runs on QEMU's `virt` machine and gives user programs threads, semaphores, sleeping, a heap allocator and buffered console I/O (input/output) through system calls.

It started as the project for the Operating Systems 1 course at the School of Electrical Engineering, University of Belgrade (ETF), which provides the build setup on an xv6 base and a small hardware library (`lib/`). Everything in `src/` and `h/` is my own.

## Features

- **Privilege separation:** the kernel runs in S-mode (Supervisor mode), user threads in U-mode (User mode). User code reaches the kernel only through `ecall`.
- **Trap handling:** one assembly entry point saves the full register state into a trap frame on the thread's kernel stack and dispatches system calls, timer interrupts, console interrupts and faults. A faulting user thread is killed; the kernel keeps running.
- **Threads:** each thread has its own user stack and kernel stack. An exited thread is freed by the next thread right after the switch away from it. Context switches happen in kernel mode and save only callee-saved registers.
- **Scheduler:** preemptive, 16 priority levels, time slices, and active/expired queues so that low-priority threads can't starve. Threads woken by a semaphore get a temporary priority boost.
- **Memory allocator:** block-based, first-fit, with $O(1)$ coalescing on free.
- **Semaphores:** counting semaphores with FIFO (first in, first out) waiters, multi-unit `wait_n` / `signal_n`, and safe closing while threads are still waiting.
- **Sleeping:** a delta list, so each timer tick does $O(1)$ work no matter how many threads are asleep.
- **Console:** interrupt-driven input and output through ring buffers, with a kernel thread that drains the output buffer.
- **APIs:** a C API (`h/syscall_c.h`) and a C++ API (`h/syscall_cpp.hpp`) with `Thread`, `PeriodicThread`, `Semaphore` and `Console` classes.

## Building and running

You need a RISC-V GCC (GNU Compiler Collection) cross-toolchain (`riscv64-unknown-elf-` or `riscv64-linux-gnu-`), `qemu-system-riscv64`, and `gdb-multiarch` for debugging. The easiest way to get all three is the Docker image in this repo:

```sh
docker build -t riscv-kernel .
docker run --rm -it -v "$PWD":/kernel riscv-kernel
```

Then, inside the container (or on a host with the toolchain installed):

```sh
make          # build the kernel image (kernel) and its disassembly (kernel.asm)
make qemu     # run it on QEMU: virt machine, 1 hart, 128 MB of RAM
make clean
```

The kernel shuts QEMU down when it finishes. If it doesn't, exit with `Ctrl-A` then `X`.

### Debugging with GDB

```sh
make qemu-gdb               # starts QEMU halted, waiting for GDB
gdb-multiarch -x .gdbinit   # in a second shell, e.g. docker exec -it <container> bash
```

`make qemu-gdb` generates `.gdbinit` from `.gdbinit.tmpl-riscv`, which connects to QEMU and loads the kernel's symbols. Kernel logging is controlled by `DEBUG_PRINT` in the `Makefile`.

## Project layout

```text
src/
  main.cpp              boot: trap vector, allocator, scheduler and console init, then starts user code
  arch/                 trap entry (asm), trap handler, context switch (asm), jump to user mode (asm), shutdown
  kernel/               kmem (allocator), kthread, ksched, ksem, ktime, kio (console)
  api/                  user-side C system call wrappers, C++ operator new/delete
  utils/                thread_pq: per-priority ready queues
h/                      headers, same structure as src/; h/arch/ has CSR (Control and Status Register) helpers
lib/                    course-provided hardware library (hw.lib, console.lib, mem.lib) and headers
kernel.ld               linker script: the kernel is loaded at 0x80000000
```

## Implementation details

### Boot

`main()` points `stvec` (Supervisor Trap Vector) at `trap_entry`, then initializes the allocator, the scheduler and the console. The boot code itself becomes a low-priority kernel thread. It creates the user program's main thread, waits for it to finish (`kthread_join`), and shuts the machine down.

### Traps and system calls

On a trap, `trap_entry` swaps `sp` with `sscratch` (which holds the current thread's kernel stack pointer while user code runs), pushes all 32 general-purpose registers plus `sepc` (Supervisor Exception Program Counter) and `sstatus` (Supervisor Status) as a `trap_frame`, and calls `trap_handler`. The handler reads `scause` (Supervisor Trap Cause):

- **`ecall` from U-mode:** the system call code is in `a0` and the arguments in `a1`–`a4`. The result goes back into `a0` of the saved frame, and `sepc` is moved past the `ecall`.
- **Timer interrupt:** advances sleeping threads and the running thread's time slice, then calls the scheduler.
- **External interrupt:** claimed from the PLIC (Platform-Level Interrupt Controller); console interrupts go to `kio`.
- **Illegal instruction, load or store fault:** the thread is terminated.

Interrupts are off while the kernel handles a trap, and there is one hart (hardware thread), so kernel code needs no locks.

System calls:

| Code | Call | Code | Call |
|---|---|---|---|
| `0x00` | `mem_alloc` | `0x22` | `sem_close` |
| `0x01` | `mem_free` | `0x23` | `sem_wait` |
| `0x11` | `thread_create` | `0x24` | `sem_signal` |
| `0x12` | `thread_exit` | `0x25` | `sem_wait_n` |
| `0x13` | `thread_dispatch` | `0x26` | `sem_signal_n` |
| `0x21` | `sem_open` | `0x31` | `time_sleep` |
| `0x41` | `getc` | `0x42` | `putc` |

Errors are returned as negative POSIX-style codes (`-ENOMEM`, `-EBADF`, `-ECANCELED`, ...), see `h/utils/errno.h`.

### Threads and context switching

A `thread` holds its saved context (`sp`, `ra`), its user and kernel stacks, its status, priority, remaining time slice, and an optional joiner. `context_switch` pushes the callee-saved registers (`s0`–`s11`, `tp`) onto the current kernel stack, saves `sp` and `ra`, and loads the next thread's. Each switch happens inside a function call in kernel mode, so the caller-saved registers are already handled by the compiler.

A new user thread starts with a kernel stack prepared so that its first switch-in "returns" into `user_entry_wrapper`. That sets `sepc` to a small wrapper around the thread body, and `user_entry_jump` sets up `sstatus` (previous mode = U, interrupts enabled on return) and executes `sret`. When the body returns, the wrapper calls `thread_exit` itself.

### Scheduling

- 16 priority levels, 0 (lowest) to 15 (highest). User threads get priority 8, the console output thread 12, the boot thread 1 and the idle thread 0.
- Ready threads sit in per-priority FIFO queues, with a 16-bit mask of non-empty levels.
- There are two sets of queues, **active** and **expired**. A thread that uses up its time slice (2 timer ticks) goes into the expired set. When the active set is empty, the two swap. A thread that keeps using up its slices waits in the expired set until every active thread has run, so high-priority threads can't starve low-priority ones forever.
- If a thread with a higher priority than the running one becomes ready, it preempts the running thread at the next timer tick or interrupt.
- **Priority boosting:** a user thread woken by `sem_signal` gets +1 priority, up to +3, until it is next scheduled. Threads that block on I/O or synchronization get the CPU (Central Processing Unit) back quickly.
- Each thread records the priority it was queued at (`queued_priority`), so a change in its boost while queued can't make it be removed from the wrong queue.
- When nothing is ready, an idle thread runs `wfi` (wait for interrupt).

### Memory allocation

The heap is split into 64-byte blocks, and `mem_alloc` (as the course spec requires) gets a number of blocks, not bytes. A normal boundary-tag allocator would put a header and a footer on each segment, but here each would take up a whole block.

The solution: one metadata block sits **between** every two segments. It works as the footer of the segment on its left and the header of the segment on its right, storing the size and free flag of both. Freeing a segment can therefore check both neighbours and merge with them in $O(1)$, handling all four cases (no free neighbour, left free, right free, both free). Free segments form a doubly linked list. Allocation is first-fit and splits a segment only when the remainder is big enough to be useful.

### Semaphores

`sem_wait_n(s, n)` takes `n` units immediately if they are available. Otherwise the thread puts a node on its own kernel stack into the semaphore's FIFO queue and blocks. `sem_signal_n` adds units and wakes waiters from the front for as long as their requests can be met. From then on, the units belong to the woken thread, so a later arrival can't take them. `sem_close` wakes every waiter with `-ECANCELED` before freeing the semaphore, and the waiters never touch it again.

### Sleeping

Sleeping threads are kept in a delta list: each node stores the number of ticks **after the previous node** wakes. A timer tick only decrements the head and wakes everything that reached zero, so a tick costs $O(1)$ plus the number of threads woken.

### Console I/O

Output and input each use a 128-byte ring buffer guarded by two semaphores (free space, available data).

- `putc` writes straight to the device when it is ready and the buffer is empty (fast path). Otherwise it queues the character. A kernel thread with priority 12 drains the buffer and blocks while the device is busy, until the device's interrupt wakes it.
- The input interrupt handler reads every available character into the input buffer and signals the semaphore once with the count. `getc` blocks until there is data.

## Known limitations

- No virtual memory: all threads share one physical address space, and a user thread can read or write kernel memory.
- Single hart only.
- There is no system call for setting a thread's priority.

## License

Built on the xv6 base provided by the course. xv6 is MIT-licensed, see [LICENSE](LICENSE).
