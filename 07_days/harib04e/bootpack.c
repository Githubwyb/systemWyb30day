#include "bootpack.h"

#include <linux/compiler.h>
#include <linux/kernel.h>

#include "asmfunc.h"
#include "fonts.h"
#include "graphic.h"

void HariMain(void) {
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;

    // 初始化段和中断
    init_gdtidt();
    init_pic();
    io_sti();  // IDT和PIC初始化完成后，解除中断禁止

    init_palette();
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

    unsigned char mcursor[256];
    int mx = (binfo->scrnx - 16) / 2;
    int my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_009999);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

    io_out8(PIC0_IMR, 0xf9);  // 11111001 允许PIC1和键盘的中断
    io_out8(PIC1_IMR, 0xef);  // 11101111 允许鼠标的中断

    for (;;) {
        io_cli();
        if (kfifo_is_empty(&g_keybuf)) {
            io_stihlt();
        } else {
            unsigned char i;
            kfifo_get(&g_keybuf, &i);
            io_sti();
            char s[4];
            sprintf(s, "%02X", i);
            boxfill8(binfo->vram, binfo->scrnx, COL8_009999, 0, 16, 15, 31);
            put_font8_str(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
        }
    }
}
