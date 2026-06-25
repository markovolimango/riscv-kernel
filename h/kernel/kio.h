#ifndef KIO_H
#define KIO_H

#ifdef __cplusplus
extern "C" {
#endif

void kio_init();
void kio_putc(char c);
void kio_puts(const char *s);
char kio_getc();
void kio_handle_console_irq();

#ifdef __cplusplus
}
#endif

#endif // KIO_H
