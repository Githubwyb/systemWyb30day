#include <linux/compiler_types.h>

#define io_cli() asm("cli")  //!!!本次添加部分
#define io_sti() asm("sti")  //!!!本次添加部分
#define io_hlt() asm("hlt")

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
