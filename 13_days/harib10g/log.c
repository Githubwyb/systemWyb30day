#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/string.h>

#include "asmfunc.h"

/* Serial functions loosely based on a similar package from Klaus P. Gerlicher */

static unsigned long early_serial_base = 0x3f8; /* ttyS0 */

#define XMTRDY 0x20

#define DLAB 0x80

#define TXR 0 /*  Transmit register (WRITE) */
#define RXR 0 /*  Receive register  (READ)  */
#define IER 1 /*  Interrupt Enable          */
#define IIR 2 /*  Interrupt ID              */
#define FCR 2 /*  FIFO control              */
#define LCR 3 /*  Line control              */
#define MCR 4 /*  Modem control             */
#define LSR 5 /*  Line Status               */
#define MSR 6 /*  Modem Status              */
#define DLL 0 /*  Divisor Latch Low         */
#define DLH 1 /*  Divisor latch High        */

static inline unsigned int io_serial_in(unsigned long addr, int offset) { return inb(addr + offset); }

static inline void io_serial_out(unsigned long addr, int offset, int value) { outb(value, addr + offset); }

static inline int serial_putc(unsigned char ch) {
    unsigned timeout = 0xffff;

    while ((io_serial_in(early_serial_base, LSR) & XMTRDY) == 0) {
        --timeout;
    }
    io_serial_out(early_serial_base, TXR, ch);
    return timeout ? 0 : -1;
}

static inline void serial_write(const char *s, unsigned n) {
    while (*s && n-- > 0) {
        if (*s == '\n') serial_putc('\r');
        serial_putc(*s);
        s++;
    }
}

void serial_printf(const char *fmt, ...) {
    va_list ap;
    char buf[512];
    int n;

    va_start(ap, fmt);
    n = vscnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    serial_write(buf, n);
}

void _print_current_time(void) {
    unsigned long msecs = jiffies_to_msecs(jiffies);
    unsigned long secs = msecs / 1000;
    unsigned long msec = msecs % 1000;
    serial_printf("[%5lu.%03lu]", secs, msec);
}
