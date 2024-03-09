#include <linux/compiler.h>

#include "asmfunc.h"
#include "bootpack.h"
#include "fonts.h"
#include "graphic.h"

struct BOOTINFO {
    char cyls, leds, vmode, reserve;
    short scrnx;
    short scrny;
    u8 *vram;
};

void HariMain(void) {
    struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;

    init_gdtidt();
    init_palette();
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

    unsigned char mcursor[256];
    int mx = (binfo->scrnx - 16) / 2;
    int my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_009999);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

    for (;;) {
        io_hlt();
    }
}
