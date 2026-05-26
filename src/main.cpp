#include "../h/arch/regs.h"
#include "../h/arch/shutdown.h"
#include "../h/kernel/kmem.h"
#include "../h/kernel/kthread.h"
#include "../h/syscall_c.h"
#include "../h/syscall_cpp.hpp"

// helper to print strings since we only have putc
static void prints(const char *s) {
    while (*s)
        putc(*s++);
}

static void print_ok(const char *test) {
    prints("[OK] ");
    prints(test);
    putc('\n');
}

static void print_fail(const char *test) {
    prints("[FAIL] ");
    prints(test);
    putc('\n');
}

// ---- TEST 1: basic create via constructor ----
static void test_create() {
    Semaphore s(3);
    // Resource cleanly cleans up at scope exit via destructor automatically.
    print_ok("create: val=3 instance initialized");

    Semaphore s0(0);
    print_ok("create: val=0 instance initialized");
}

// ---- TEST 2: wait without blocking ----
static void test_wait_no_block() {
    Semaphore s(3);

    int r1 = s.wait();
    int r2 = s.wait(); // Note: The C++ wrapper doesn't expose wait_n, using sequential waits

    if (r1 == 0 && r2 == 0)
        print_ok("wait: consumed resources smoothly without blocking");
    else
        print_fail("wait: unexpected blocking or error");
}

// ---- TEST 3: signal without waiters ----
static void test_signal_no_waiters() {
    Semaphore s(0);

    int r1 = s.signal();
    int r2 = s.signal(); // Using sequential signals as signal_n is hidden by the wrapper

    if (r1 == 0 && r2 == 0)
        print_ok("signal: released resources without active waiters");
    else
        print_fail("signal: failed execution");
}

// ---- TEST 4: signal unblocks a waiting thread ----
static Semaphore *test4_sem = nullptr;
static volatile int test4_order = 0;

static void test4_waiter(void *arg) {
    if (test4_sem) {
        test4_sem->wait();
    }
    if (test4_order == 1)
        print_ok("signal/wait: waiter unblocked after signal");
    else
        print_fail("signal/wait: waiter unblocked in wrong order");
    test4_order = 2;
}

static void test_signal_unblocks() {
    test4_sem = new Semaphore(0);
    test4_order = 0;

    Thread t(test4_waiter, nullptr);
    t.start(); // Moves thread from initialized to ready queue

    Thread::dispatch(); // give waiter cpu, it blocks on sem
    test4_order = 1;
    test4_sem->signal(); // unblock waiter
    Thread::dispatch();  // let waiter finish execution

    if (test4_order == 2)
        print_ok("signal/wait: main resumed after waiter");
    else
        print_fail("signal/wait: main resumed after waiter");

    delete test4_sem;
    test4_sem = nullptr;
}

// ---- TEST 5: close (destructor) unblocks waiters with error ----
static Semaphore *test5_sem = nullptr;
static volatile int test5_got_error = 0;

static void test5_waiter(void *arg) {
    if (test5_sem) {
        int ret = test5_sem->wait();
        // The destructor calls sem_close, waking up threads with an error token (!= 0)
        if (ret != 0) {
            test5_got_error = 1;
        }
    }
}

static void test_close_unblocks() {
    test5_sem = new Semaphore(0);
    test5_got_error = 0;

    Thread t(test5_waiter, nullptr);
    t.start();

    Thread::dispatch(); // let waiter block
    delete test5_sem;   // Invokes ~Semaphore(), explicitly destroying internal kernel semaphore
    test5_sem = nullptr;

    Thread::dispatch(); // let waiter run post-destruction to catch ret val

    if (test5_got_error)
        print_ok("close: waiter unblocked via destructor and caught error");
    else
        print_fail("close: waiter failed to gracefully drop on object destruction");
}

// ---- TEST 6: multiple waiters, signal wakes in FIFO order ----
static Semaphore *test6_sem = nullptr;
static volatile int test6_sequence = 0;

static void test6_waiter_a(void *arg) {
    if (test6_sem)
        test6_sem->wait();
    if (test6_sequence == 0)
        print_ok("fifo: waiter A woke first");
    else
        print_fail("fifo: waiter A woke out of order");
    test6_sequence++;
}

static void test6_waiter_b(void *arg) {
    if (test6_sem)
        test6_sem->wait();
    if (test6_sequence == 1)
        print_ok("fifo: waiter B woke second");
    else
        print_fail("fifo: waiter B woke out of order");
    test6_sequence++;
}

static void test_fifo_order() {
    test6_sem = new Semaphore(0);
    test6_sequence = 0;

    Thread ta(test6_waiter_a, nullptr);
    Thread tb(test6_waiter_b, nullptr);
    ta.start();
    tb.start();

    Thread::dispatch();  // Thread A executes and blocks
    Thread::dispatch();  // Thread B executes and blocks
    test6_sem->signal(); // wake head of queue (Thread A)
    Thread::dispatch();  // Thread A processes
    test6_sem->signal(); // wake next in line (Thread B)
    Thread::dispatch();  // Thread B processes

    delete test6_sem;
    test6_sem = nullptr;
}

void run_sem_tests() {
    prints("=== ksem C++ API tests ===\n");
    test_create();
    test_wait_no_block();
    test_signal_no_waiters();
    test_signal_unblocks();
    test_close_unblocks();
    test_fifo_order();
    prints("=== done ===\n");
}

// Infrastructure hooks
extern "C" void trap_entry();
extern void stvec_write(uint64);
extern void kmem_init();
extern void kthread_init();
extern void kmem_dump();
extern void shutdown(const char *);

void main() {
    stvec_write((uint64)trap_entry);
    kmem_init();
    kthread_init();

    run_sem_tests();

    kmem_dump();
    putc('\n');

    shutdown("Execution complete");
}
