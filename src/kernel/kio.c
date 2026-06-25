#include "../../h/kernel/kio.h"
#include "../../h/arch/shutdown.h"
#include "../../h/kernel/ksem.h"
#include "../../lib/hw.h"

static inline uint8 tx_ready() { return *(uint8 *)CONSOLE_STATUS & CONSOLE_TX_STATUS_BIT; }

static inline uint8 rx_ready() { return *(uint8 *)CONSOLE_STATUS & CONSOLE_RX_STATUS_BIT; }

#define IO_BUF_SIZE 128

typedef struct {
    char buf[IO_BUF_SIZE];
    uint8 head;
    uint8 tail;
    sem *space;
    sem *data;
} io_buf;

static void io_buf_init(io_buf *buf) {
    buf->head = buf->tail = 0;
    buf->space = ksem_create(IO_BUF_SIZE);
    buf->data = ksem_create(0);
    if (!buf->space || !buf->data) shutdown();
}

static void io_buf_put(io_buf *buf, char c) {
    buf->buf[buf->tail] = c;
    buf->tail = (buf->tail + 1) % IO_BUF_SIZE;
}

static char io_buf_take(io_buf *buf) {
    char c = buf->buf[buf->head];
    buf->head = (buf->head + 1) % IO_BUF_SIZE;
    return c;
}

static io_buf tx_buf;
static io_buf rx_buf;
static thread *tx_thread;
static uint8 tx_hw_blocked = 1;

static void tx_thread_body(void *arg) {
    while (1) {
        ksem_wait(tx_buf.data);
        if (!tx_ready()) {
            tx_hw_blocked = 1;
            kthread_block();
        }
        *(uint8 *)CONSOLE_TX_DATA = io_buf_take(&tx_buf);
        ksem_signal(tx_buf.space);
    }
}

void kio_init() {
    io_buf_init(&tx_buf);
    io_buf_init(&rx_buf);
    tx_thread = kthread_create_kernel(tx_thread_body, 0, 12);
}

void kio_putc(char c) {
    if (tx_ready() && tx_buf.head == tx_buf.tail) {
        *(uint8 *)CONSOLE_TX_DATA = c;
        return;
    }
    ksem_wait(tx_buf.space);
    io_buf_put(&tx_buf, c);
    ksem_signal(tx_buf.data);
    ksched_switch();
}

void kio_puts(const char *s) {
    while (*s) kio_putc(*s++);
}

char kio_getc() {
    ksem_wait(rx_buf.data);
    return io_buf_take(&rx_buf);
}

static inline void handle_tx_irq() {
    tx_hw_blocked = 0;
    kthread_unblock(tx_thread);
}

static inline void handle_rx_irq() {
    uint8 count = 0;
    while (rx_ready()) {
        char c = (char)(*(uint8 *)CONSOLE_RX_DATA);
        io_buf_put(&rx_buf, c);
        count++;
    }
    ksem_signal_n(rx_buf.data, count);
}

void kio_handle_console_irq() {
    if (tx_ready() && tx_hw_blocked) handle_tx_irq();
    if (rx_ready()) handle_rx_irq();
}
