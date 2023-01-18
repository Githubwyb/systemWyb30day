#include "bootpack.h"

#include <linux/stddef.h>
#include <linux/types.h>
#include <stdio.h>
#include <string.h>

#include "fonts.h"
#include "graphic.h"
#include "naskfunc.h"

void HariMain(void) {
    char s[40];

    // 初始化段和中断
    init_gdtidt();
    init_pic();
    io_sti();                 // IDT和PIC初始化完成后，解除中断禁止
    io_out8(PIC0_IMR, 0xf9);  // 11111001 允许PIC1和键盘的中断
    io_out8(PIC1_IMR, 0xef);  // 11101111 允许鼠标的中断

    init_keyboard();
    enable_mouse();
    uint32_t memtotal = memtest(0x00400000, 0xbfffffff);
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000);
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    init_palette();
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

    unsigned char mcursor[256];
    int mx = (binfo->scrnx - 16) / 2;
    int my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_009999);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

    sprintf(s, "memory %dMB    free: %dKB", memtotal / 1024 / 1024, memman_total(memman) / 1024);
    put_font8_str(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

    struct MOUSE_DEC mdec;
    mdec.phase = 0;
    for (;;) {
        io_cli();
        if (kfifo_is_empty(&g_keybuf) && kfifo_is_empty(&g_mouseBuf)) {
            io_stihlt();
        } else {
            unsigned char i;
            if (!kfifo_is_empty(&g_keybuf)) {
                kfifo_get(&g_keybuf, &i);
                io_sti();
                sprintf(s, "%02X", i);
                boxfill8(binfo->vram, binfo->scrnx, COL8_009999, 0, 16, 15, 31);
                put_font8_str(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
            } else if (!kfifo_is_empty(&g_mouseBuf)) {
                kfifo_get(&g_mouseBuf, &i);
                io_sti();

                if (!mouse_decode(&mdec, i)) {
                    continue;
                }

                // 鼠标数据解读完成，显示画面
                sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
                if ((mdec.btn & 0x01) != 0) {
                    s[1] = 'L';
                }
                if ((mdec.btn & 0x02) != 0) {
                    s[3] = 'R';
                }
                if ((mdec.btn & 0x04) != 0) {
                    s[2] = 'C';
                }
                // 打印鼠标数据
                boxfill8(binfo->vram, binfo->scrnx, COL8_009999, 32, 16, 32 + 15 * 8 - 1, 31);
                put_font8_str(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);

                // 移动光标
                // 当前光标位置涂成背景色
                boxfill8(binfo->vram, binfo->scrnx, COL8_009999, mx, my, mx + 15, my + 15);
                mx += mdec.x;
                my += mdec.y;
                // 限定边界
                if (mx < 0) {
                    mx = 0;
                }
                if (my < 0) {
                    my = 0;
                }
                if (mx > binfo->scrnx - 16) {
                    mx = binfo->scrnx - 16;
                }
                if (my > binfo->scrny - 16) {
                    my = binfo->scrny - 16;
                }
                sprintf(s, "(%3d, %3d)", mx, my);
                boxfill8(binfo->vram, binfo->scrnx, COL8_009999, 0, 0, 79, 15);
                put_font8_str(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
                // 把光标画上去
                putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
            }
        }
    }
}
