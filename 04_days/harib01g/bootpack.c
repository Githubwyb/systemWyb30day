#include "color.h"

// 声明外部汇编函数
extern void io_hlt(void);
extern void io_cli(void);
extern void io_out8(int port, int data);
extern int  io_load_eflags(void);
extern void io_store_eflags(int eflags);

static void set_palette(int start, int end, unsigned char *rgb);

void init_palette() {
    set_palette(0, 255, table_rgb);
}

void set_palette(int start, int end, unsigned char *rgb) {
    int i, eflags;
    eflags = io_load_eflags(); /* 记录终端许可标志的值 */
    io_cli();                  /* 将许可标志置0，禁止中断 */
    io_out8(0x03c8, start);
    for (i = start; i <= end; i++) {
        io_out8(0x03c9, rgb[0] / 4);
        io_out8(0x03c9, rgb[1] / 4);
        io_out8(0x03c9, rgb[2] / 4);
        rgb += 3;
    }
    io_store_eflags(eflags); /* 恢复许可标志 */
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1) {
    int x, y;
    for (x = x0; x <= x1; ++x) {
        for (y = y0; y <= y1; ++y) {
            vram[y * xsize + x] = c;
        }
    }
}

void HariMain(void) {
    char *p;

    init_palette();
    p = (char *)0xa0000;

    boxfill8(p, 320, COL8_FF0000, 20, 20, 120, 120);
    boxfill8(p, 320, COL8_00FF00, 70, 50, 170, 150);
    boxfill8(p, 320, COL8_0000FF, 120, 80, 220, 180);

    for (;;) {
        io_hlt();
    }
}
