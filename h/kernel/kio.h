#ifndef KIO_H
#define KIO_H

#ifdef __cplusplus
extern "C" {
#endif

void kio_init();
void kputc(char c);
char kgetc();
void kio_handle_console_irq();

#ifdef __cplusplus
}
#endif

#endif // KIO_H
