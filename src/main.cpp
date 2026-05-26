#include "../h/kmem.h"
#include "../h/ksem.h"
#include "../h/kthread.h"
#include "../h/regs.h"
#include "../h/shutdown.h"
#include "../h/syscall_c.h"

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

// ---- TEST 1: basic create, val is correct ----
static void test_create() {
    sem *s = ksem_create(3);
    if (s && s->val == 3 && s->head == 0 && s->tail == 0)
        print_ok("create: val=3, empty queue");
    else
        print_fail("create: val=3, empty queue");

    sem *s0 = ksem_create(0);
    if (s0 && s0->val == 0)
        print_ok("create: val=0");
    else
        print_fail("create: val=0");

    ksem_close(s);
    ksem_close(s0);
}

// ---- TEST 2: wait without blocking, val decrements correctly ----
static void test_wait_no_block() {
    sem *s = ksem_create(3);
    ksem_wait(s);
    if (s->val == 2)
        print_ok("wait: val 3->2");
    else
        print_fail("wait: val 3->2");

    ksem_wait_n(s, 2);
    if (s->val == 0)
        print_ok("wait_n: val 2->0");
    else
        print_fail("wait_n: val 2->0");

    ksem_close(s);
}

// ---- TEST 3: signal without waiters, val increments correctly ----
static void test_signal_no_waiters() {
    sem *s = ksem_create(0);
    ksem_signal(s);
    if (s->val == 1)
        print_ok("signal: val 0->1");
    else
        print_fail("signal: val 0->1");

    ksem_signal_n(s, 4);
    if (s->val == 5)
        print_ok("signal_n: val 1->5");
    else
        print_fail("signal_n: val 1->5");

    ksem_close(s);
}

// ---- TEST 4: signal unblocks a waiting thread ----
static sem *test4_sem;
static int test4_order = 0;

static void test4_waiter(void *arg) {
    ksem_wait(test4_sem);
    if (test4_order == 1)
        print_ok("signal/wait: waiter unblocked after signal");
    else
        print_fail("signal/wait: waiter unblocked in wrong order");
    test4_order = 2;
    kthread_exit();
}

static void test_signal_unblocks() {
    test4_sem = ksem_create(0);
    test4_order = 0;
    kmem_dump();
    void *stack = kmem_alloc(DEFAULT_STACK_SIZE);
    if (!stack)
        shutdown("no stack");
    kthread_create(test4_waiter, 0, stack);
    // waiter is in scheduler but hasn't run yet
    kthread_dispatch(); // give waiter cpu, it blocks on sem
    test4_order = 1;
    ksem_signal(test4_sem); // unblock waiter
    kthread_dispatch();     // let waiter run
    if (test4_order == 2)
        print_ok("signal/wait: main resumed after waiter");
    else
        print_fail("signal/wait: main resumed after waiter");
    ksem_close(test4_sem);
}

// ---- TEST 5: close unblocks waiters with error ----
static sem *test5_sem;
static int test5_got_error = 0;

static void test5_waiter(void *arg) {
    int ret = ksem_wait(test5_sem);
    if (ret != 0)
        test5_got_error = 1;
    kthread_exit();
}

static void test_close_unblocks() {
    test5_sem = ksem_create(0);
    void *stack = kmem_alloc(DEFAULT_STACK_SIZE);
    kthread_create(test5_waiter, 0, stack);
    kthread_dispatch();    // let waiter block
    ksem_close(test5_sem); // should wake waiter with error
    kthread_dispatch();    // let waiter run and check ret
    if (test5_got_error)
        print_ok("close: waiter got error return");
    else
        print_fail("close: waiter got error return");
}

// ---- TEST 6: multiple waiters, signal wakes in FIFO order ----
static sem *test6_sem;
static int test6_sequence = 0;

static void test6_waiter_a(void *arg) {
    ksem_wait(test6_sem);
    if (test6_sequence == 0)
        print_ok("fifo: waiter A woke first");
    else
        print_fail("fifo: waiter A woke out of order");
    test6_sequence++;
    kthread_exit();
}

static void test6_waiter_b(void *arg) {
    ksem_wait(test6_sem);
    if (test6_sequence == 1)
        print_ok("fifo: waiter B woke second");
    else
        print_fail("fifo: waiter B woke out of order");
    test6_sequence++;
    kthread_exit();
}

static void test_fifo_order() {
    test6_sem = ksem_create(0);
    test6_sequence = 0;
    void *stack_a = kmem_alloc(DEFAULT_STACK_SIZE);
    void *stack_b = kmem_alloc(DEFAULT_STACK_SIZE);
    kthread_create(test6_waiter_a, 0, stack_a);
    kthread_create(test6_waiter_b, 0, stack_b);
    kthread_dispatch();     // a blocks
    kthread_dispatch();     // b blocks
    ksem_signal(test6_sem); // wake a
    kthread_dispatch();     // a runs
    ksem_signal(test6_sem); // wake b
    kthread_dispatch();     // b runs
    ksem_close(test6_sem);
}

// ---- TEST 7: wait_n blocks until enough resources ----
static sem *test7_sem;
static int test7_done = 0;

static void test7_waiter(void *arg) {
    int ret = ksem_wait_n(test7_sem, 3);
    if (ret == 0 && test7_sem->val == 0)
        print_ok("wait_n: blocked until 3 resources available");
    else
        print_fail("wait_n: wrong behavior waiting for 3 resources");
    test7_done = 1;
    kthread_exit();
}

static void test_wait_n_blocks() {
    test7_sem = ksem_create(0);
    test7_done = 0;
    void *stack = kmem_alloc(DEFAULT_STACK_SIZE);
    kthread_create(test7_waiter, 0, stack);
    kthread_dispatch();     // waiter blocks, needs 3
    ksem_signal(test7_sem); // val=1, not enough
    ksem_signal(test7_sem); // val=2, not enough
    if (test7_done)
        print_fail("wait_n: woke too early");
    ksem_signal(test7_sem); // val=3, enough, should unblock
    kthread_dispatch();
    if (test7_done)
        print_ok("wait_n: woke at right time");
    else
        print_fail("wait_n: never woke");
    ksem_close(test7_sem);
}

void run_sem_tests() {
    prints("=== ksem tests ===\n");
    test_create();
    test_wait_no_block();
    test_signal_no_waiters();
    test_signal_unblocks();
    test_close_unblocks();
    test_fifo_order();
    test_wait_n_blocks();
    prints("=== done ===\n");
}

extern "C" void trap_entry();

extern void userMain();

void b1(void *arg) { putc('1'); }

void main() {
    stvec_write((uint64)trap_entry);
    kmem_init();
    kthread_init();

    run_sem_tests();

    kmem_dump();

    putc('\n');

    shutdown("Execution complete");
}
