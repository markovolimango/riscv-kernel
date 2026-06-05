#include "../../h/kernel/kio.h"
#include "../../h/kernel/ksem.h"
#include "../../lib/hw.h"

#define IO_BUF_SIZE 128

typedef struct {
    char buf[IO_BUF_SIZE];
    unsigned head;
    unsigned tail;
    sem *space;
    sem *data;
} io_buf;

static void io_buf_put(io_buf *buf, char c) {
    buf->buf[buf->tail] = c;
    buf->tail = (buf->tail + 1) % IO_BUF_SIZE;
}

static char io_buf_get(io_buf *buf) {
    char c = buf->buf[buf->head];
    buf->head = (buf->head + 1) % IO_BUF_SIZE;
    return c;
}

static io_buf tx_buf;
static io_buf rx_buf;

void tx_thread_body(void *arg) {
    while (1) {
        ksem_wait(tx_buf.data);
        char c = io_buf_get(&tx_buf);
        while (!(*(uint8 *)CONSOLE_STATUS & (1 << 5)))
            ;
        *(uint8 *)CONSOLE_TX_DATA = c;
        ksem_signal(tx_buf.space);
    }
}

void kio_init() {
    tx_buf.head = tx_buf.tail = 0;
    tx_buf.space = ksem_create(IO_BUF_SIZE);
    tx_buf.data = ksem_create(0);
    tcb *tx_thread = kthread_create(tx_thread_body, 0, 1);
    tx_thread->priority = 0;

    rx_buf.head = rx_buf.tail = 0;
    rx_buf.space = ksem_create(IO_BUF_SIZE);
    rx_buf.data = ksem_create(0);
}

void kputc(char c) {
    ksem_wait(tx_buf.space);
    io_buf_put(&tx_buf, c);
    ksem_signal(tx_buf.data);
}

char kgetc() {
    ksem_wait(rx_buf.data);
    char c = io_buf_get(&rx_buf);
    ksem_signal(rx_buf.space);
    return c;
}

void kio_handle_console_irq() {
    uint8 status = *(uint8 *)CONSOLE_STATUS;
    if (!(status & CONSOLE_RX_STATUS_BIT)) return;
    char c = *(uint8 *)CONSOLE_RX_DATA;
    if (rx_buf.data->val < IO_BUF_SIZE) {
        io_buf_put(&rx_buf, c);
        ksem_signal(rx_buf.data);
    }
}
