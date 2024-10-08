#include <linux/compiler.h>

#include <linux/kernel.h>

#include "asmfunc.h"
#include "color.h"
#include "fonts.h"

static void set_palette(int start, int end, unsigned char *rgb);

void init_palette() { set_palette(0, 255, table_rgb); }

void set_palette(int start, int end, unsigned char *rgb) {
    int i, eflags;
    eflags = io_load_eflags(); /* 记录中断许可标志的值 */
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

struct BOOTINFO {
    char cyls, leds, vmode, reserve;
    short scrnx;
    short scrny;
    u8 *vram;
};

static void init_screen(u8 *vram, int xsize, int ysize) {
    boxfill8(vram, xsize, COL8_009999, 0, 0, xsize - 1, ysize - 29);
    boxfill8(vram, xsize, COL8_CCCCCC, 0, ysize - 28, xsize - 1, ysize - 28);
    boxfill8(vram, xsize, COL8_FFFFFF, 0, ysize - 27, xsize - 1, ysize - 27);
    boxfill8(vram, xsize, COL8_CCCCCC, 0, ysize - 26, xsize - 1, ysize - 1);

    boxfill8(vram, xsize, COL8_FFFFFF, 3, ysize - 24, 59, ysize - 24);
    boxfill8(vram, xsize, COL8_FFFFFF, 2, ysize - 24, 2, ysize - 4);
    boxfill8(vram, xsize, COL8_999999, 3, ysize - 4, 59, ysize - 4);
    boxfill8(vram, xsize, COL8_999999, 59, ysize - 23, 59, ysize - 5);
    boxfill8(vram, xsize, COL8_000000, 2, ysize - 3, 59, ysize - 3);
    boxfill8(vram, xsize, COL8_000000, 60, ysize - 24, 60, ysize - 3);

    boxfill8(vram, xsize, COL8_999999, xsize - 47, ysize - 24, xsize - 4, ysize - 24);
    boxfill8(vram, xsize, COL8_999999, xsize - 47, ysize - 23, xsize - 47, ysize - 4);
    boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize - 3, xsize - 4, ysize - 3);
    boxfill8(vram, xsize, COL8_FFFFFF, xsize - 3, ysize - 24, xsize - 3, ysize - 3);

    char s[50];
    put_font8_str(vram, xsize, 8, 8, COL8_FFFFFF, "ABC 123");
    put_font8_str(vram, xsize, 31, 31, COL8_000000, "Hello OS.");
    put_font8_str(vram, xsize, 30, 30, COL8_FFFFFF, "Hello OS.");
    sprintf(s, "scrnx = %d", xsize);
    put_font8_str(vram, xsize, 8, 46, COL8_FFFFFF, s);
}

void HariMain(void) {
    struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;

    init_palette();
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

    for (;;) {
        io_hlt();
    }
}
