#include "bootpack.h"

#include <linux/compiler.h>
#include <linux/kernel.h>

#include "asmfunc.h"
#include "fonts.h"
#include "graphic.h"

void HariMain(void) {
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    char s[40];

    // 初始化段和中断
    init_gdtidt();
    init_pic();
    io_sti();                 // IDT和PIC初始化完成后，解除中断禁止
    io_out8(PIC0_IMR, 0xf9);  // 11111001 允许PIC1和键盘的中断
    io_out8(PIC1_IMR, 0xef);  // 11101111 允许鼠标的中断

    init_keyboard();
    enable_mouse();

    init_palette();

    // 处理内存
    uint32_t memtotal = memtest(0x00400000, 0xbfffffff);
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000);
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    struct SHTCTL *shtctl;
    struct SHEET *sht_back, *sht_mouse;
    unsigned char *buf_back, buf_mouse[256];

    /********** 初始化背景 **********/
    shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
    sht_back = sheet_alloc(shtctl);
    buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
    init_screen(buf_back, binfo->scrnx, binfo->scrny);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);  // 不存在透明色
    sheet_slide(shtctl, sht_back, 0, 0);                               // 把背景放到左上角
    sheet_updown(shtctl, sht_back, 0);                                 // 把背景放到最底层

    /********** 初始化鼠标 **********/
    sht_mouse = sheet_alloc(shtctl);
    init_mouse_cursor8(buf_mouse, 99);               // 99是透明色
    sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);  // 99是透明色
    // 先把鼠标画到正中间
    int mx = (binfo->scrnx - 16) / 2;
    int my = (binfo->scrny - 28 - 16) / 2;
    sheet_slide(shtctl, sht_mouse, mx, my);
    sheet_updown(shtctl, sht_mouse, 1);  // 把鼠标放到后面

    // 在背景上打印内存信息和鼠标位置
    sprintf(s, "(%3d, %3d)", mx, my);
    put_font8_str(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
    sprintf(s, "memory %dMB    free: %dKB", memtotal / 1024 / 1024, memman_total(memman) / 1024);
    put_font8_str(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
    sheet_refresh(shtctl, sht_back, 0, 0, binfo->scrnx, 48);  // 刷新打印字符的位置，从0,0开始，到scrnx,48结束

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
                boxfill8(buf_back, binfo->scrnx, COL8_009999, 0, 16, 15, 31);
                put_font8_str(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
                sheet_refresh(shtctl, sht_back, 0, 0, 16, 32);  // 只变更16x32的区域，和boxfill8的区域一致
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
                boxfill8(buf_back, binfo->scrnx, COL8_009999, 32, 16, 32 + 20 * 8 - 1, 31);
                put_font8_str(buf_back, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
                sheet_refresh(shtctl, sht_back, 32, 16, 32 + 20 * 8, 32);  // 和boxfill8的区域一致

                // 移动光标
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
                boxfill8(buf_back, binfo->scrnx, COL8_009999, 0, 0, 79, 15);
                put_font8_str(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
                sheet_refresh(shtctl, sht_back, 0, 0, 80, 16);  // 和boxfill8的区域一致

                // 把光标画上去
                sheet_slide(shtctl, sht_mouse, mx, my);
            }
        }
    }
}

void debug_print(const char *s) {
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    // 320 x 200
    boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 100, 200, 150);
    put_font8_str(binfo->vram, binfo->scrnx, 0, 100, COL8_FFFFFF, s);
}
