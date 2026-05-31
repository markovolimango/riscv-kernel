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

static io_buf tx_buf;

void tx_thread_body(void *arg) {
    while (1) {
        ksem_wait(tx_buf.data);
        char c = tx_buf.buf[tx_buf.head];
        tx_buf.head = (tx_buf.head + 1) % IO_BUF_SIZE;
        while (!(*(uint8 *)CONSOLE_STATUS & (1 << 5)))
            ;
        *(uint8 *)CONSOLE_TX_DATA = c;
        ksem_signal(tx_buf.space);
    }
}

void kio_init() {
    tx_buf.space = ksem_create(IO_BUF_SIZE);
    tx_buf.data = ksem_create(0);
    kthread_create(tx_thread_body, 0, 1);
}

void kputc(char c) {
    ksem_wait(tx_buf.space);
    tx_buf.buf[tx_buf.tail] = c;
    tx_buf.tail = (tx_buf.tail + 1) % IO_BUF_SIZE;
    ksem_signal(tx_buf.data);
}
