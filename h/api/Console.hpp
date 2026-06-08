#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include "../syscall_c.h"

class Console {
  public:
    static char getc();
    static void putc(char);
};

inline char Console::getc() { return ::getc(); }

inline void Console::putc(char c) { ::putc(c); }

#endif
