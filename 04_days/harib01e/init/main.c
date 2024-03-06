#include <linux/compiler_types.h>

extern void io_hlt(void);

__visible void start_kernel(void) {
    int i;
    char *p;

    p = (char *)0xa0000;
    for (i = 0x00000; i < 0x0ffff; ++i) {
        p[i] = i & 0x0f;
    }

    for (;;) {
        io_hlt();
    }
}
