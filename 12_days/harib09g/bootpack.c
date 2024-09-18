#include "bootpack.h"

#include <linux/compiler.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>

#include "asmfunc.h"
#include "fonts.h"
#include "graphic.h"

void make_window8(unsigned char *buf, int xsize, int ysize, char *title);

static struct SHEET *s_sht_back = NULL;

void HariMain(void) {
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    char s[40];

    // 初始化段和中断
    init_gdtidt();
    init_pic();
    io_sti();  // IDT和PIC初始化完成后，解除中断禁止
    init_pit();
    io_out8(PIC0_IMR, 0xf8);  // 11111000 允许PIC1、键盘、定时器的中断
    io_out8(PIC1_IMR, 0xef);  // 11101111 允许鼠标的中断

    // 处理定时器
    TimerBufType timerfifo;
    TimerBufType timerfifo2;
    TimerBufType timerfifo3;
    struct TIMER timer, timer2, timer3;
    timer_init(&timer, &timerfifo, 1);
    timer_settime(&timer, jiffies + msecs_to_jiffies(10000));
    timer_init(&timer2, &timerfifo2, 1);
    timer_settime(&timer2, jiffies + msecs_to_jiffies(3000));
    timer_init(&timer3, &timerfifo3, 1);
    timer_settime(&timer3, jiffies + msecs_to_jiffies(500));

    // 键盘鼠标
    init_keyboard();
    enable_mouse();

    init_palette();

    // 处理内存
    uint32_t memtotal = memtest(0x00400000, 0xbfffffff);
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000);
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    struct SHTCTL *shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);

    /********** 初始化背景 **********/
    struct SHEET *sht_back = sheet_alloc(shtctl);
    s_sht_back = sht_back;
    unsigned char *buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
    init_screen(buf_back, binfo->scrnx, binfo->scrny);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);  // 不存在透明色
    sheet_slide(sht_back, 0, 0);                                       // 把背景放到左上角
    sheet_updown(sht_back, 0);                                         // 把背景放到最底层0

    /********** 初始化窗口 **********/
    struct SHEET *sht_win = sheet_alloc(shtctl);
    unsigned char *buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 52);
    make_window8(buf_win, 160, 52, "counter");
    sheet_setbuf(sht_win, buf_win, 160, 52, -1);  // 没有透明色
    sheet_slide(sht_win, 80, 72);
    sheet_updown(sht_win, 1);  // 把窗口放到1层

    /********** 初始化鼠标 **********/
    struct SHEET *sht_mouse = sheet_alloc(shtctl);
    unsigned char buf_mouse[256];
    init_mouse_cursor8(buf_mouse, 99);               // 99是透明色
    sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);  // 99是透明色
    // 先把鼠标画到正中间
    int mx = (binfo->scrnx - 16) / 2;
    int my = (binfo->scrny - 28 - 16) / 2;
    sheet_slide(sht_mouse, mx, my);
    sheet_updown(sht_mouse, 2);  // 把鼠标放到2层

    // 在背景上打印内存信息和鼠标位置
    sprintf(s, "(%3d, %3d)", mx, my);
    put_font8_str(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
    sprintf(s, "memory %dMB    free: %dKB", memtotal / 1024 / 1024, memman_total(memman) / 1024);
    put_font8_str(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
    sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);  // 刷新打印字符的位置，从0,0开始，到scrnx,48结束

    struct MOUSE_DEC mdec;
    mdec.phase = 0;
    for (;;) {
        sprintf(s, "%010lu", jiffies);
        boxfill8(buf_win, 160, COL8_CCCCCC, 40, 28, 119, 43);
        put_font8_str(buf_win, 160, 40, 28, COL8_FFFFFF, s);
        sheet_refresh(sht_win, 40, 28, 160, 44);

        io_cli();
        if (kfifo_is_empty(&g_keybuf) && kfifo_is_empty(&g_mouseBuf) && kfifo_is_empty(&timerfifo) &&
            kfifo_is_empty(&timerfifo2) && kfifo_is_empty(&timerfifo3)) {
            io_sti();
            continue;
        }

        unsigned char i;
        if (!kfifo_is_empty(&g_keybuf)) {
            kfifo_get(&g_keybuf, &i);
            io_sti();
            sprintf(s, "%02X", i);
            boxfill8(buf_back, binfo->scrnx, COL8_009999, 0, 16, 15, 31);
            put_font8_str(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
            sheet_refresh(sht_back, 0, 0, 16, 32);  // 只变更16x32的区域，和boxfill8的区域一致
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
            sheet_refresh(sht_back, 32, 16, 32 + 20 * 8, 32);  // 和boxfill8的区域一致

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
            if (mx > binfo->scrnx - 1) {
                mx = binfo->scrnx - 1;
            }
            if (my > binfo->scrny - 1) {
                my = binfo->scrny - 1;
            }
            sprintf(s, "(%3d, %3d)", mx, my);
            boxfill8(buf_back, binfo->scrnx, COL8_009999, 0, 0, 79, 15);
            put_font8_str(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
            sheet_refresh(sht_back, 0, 0, 80, 16);  // 和boxfill8的区域一致

            // 把光标画上去
            sheet_slide(sht_mouse, mx, my);
        } else if (!kfifo_is_empty(&timerfifo)) {
            kfifo_get(&timerfifo, &i);
            io_sti();
            put_font8_str(buf_back, binfo->scrnx, 0, 64, COL8_FFFFFF, "10[sec]");
            sheet_refresh(sht_back, 0, 64, 56, 80);
        } else if (!kfifo_is_empty(&timerfifo2)) {
            kfifo_get(&timerfifo2, &i);
            io_sti();
            put_font8_str(buf_back, binfo->scrnx, 0, 80, COL8_FFFFFF, "3[sec]");
            sheet_refresh(sht_back, 0, 80, 48, 96);
        } else if (!kfifo_is_empty(&timerfifo3)) {
            // 模拟光标
            kfifo_get(&timerfifo3, &i);
            io_sti();
            if (i != 0) {
                timer_init(&timer3, &timerfifo3, 0);
                boxfill8(buf_back, binfo->scrnx, COL8_FFFFFF, 8, 96, 15, 111);
            } else {
                timer_init(&timer3, &timerfifo3, 1);
                boxfill8(buf_back, binfo->scrnx, COL8_009999, 8, 96, 15, 111);
            }
            timer_settime(&timer3, jiffies + msecs_to_jiffies(500));
            sheet_refresh(sht_back, 8, 96, 16, 112);
        }
    }
}

void make_window8(unsigned char *buf, int xsize, int ysize, char *title) {
    static char closebtn[14][16] = {"OOOOOOOOOOOOOOO@",
                                    "OQQQQQQQQQQQQQ$@",
                                    "OQQQQQQQQQQQQQ$@",
                                    "OQQQ@@QQQQ@@QQ$@",
                                    "OQQQQ@@QQ@@QQQ$@",
                                    "OQQQQQ@@@@QQQQ$@",
                                    "OQQQQQQ@@QQQQQ$@",
                                    "OQQQQQ@@@@QQQQ$@",
                                    "OQQQQ@@QQ@@QQQ$@",
                                    "OQQQ@@QQQQ@@QQ$@",
                                    "OQQQQQQQQQQQQQ$@",
                                    "OQQQQQQQQQQQQQ$@",
                                    "O$$$$$$$$$$$$$$@",
                                    "@@@@@@@@@@@@@@@@"};
    int x, y;
    char c;
    boxfill8(buf, xsize, COL8_CCCCCC, 0, 0, xsize - 1, 0);
    boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize - 2, 1);
    boxfill8(buf, xsize, COL8_CCCCCC, 0, 0, 0, ysize - 1);
    boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize - 2);
    boxfill8(buf, xsize, COL8_999999, xsize - 2, 1, xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
    boxfill8(buf, xsize, COL8_CCCCCC, 2, 2, xsize - 3, ysize - 3);
    boxfill8(buf, xsize, COL8_000099, 3, 3, xsize - 4, 20);
    boxfill8(buf, xsize, COL8_999999, 1, ysize - 2, xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
    put_font8_str(buf, xsize, 24, 4, COL8_FFFFFF, title);
    for (y = 0; y < 14; y++) {
        for (x = 0; x < 16; x++) {
            c = closebtn[y][x];
            if (c == '@') {
                c = COL8_000000;
            } else if (c == '$') {
                c = COL8_999999;
            } else if (c == 'Q') {
                c = COL8_CCCCCC;
            } else {
                c = COL8_FFFFFF;
            }
            buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
        }
    }
    return;
}

void debug_print(const char *fmt, ...) {
    // 320 x 200
    char s[40];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(s, fmt, ap);
    va_end(ap);
    boxfill8(s_sht_back->buf, s_sht_back->bxsize, COL8_000000, 0, 120, 200, 136);
    put_font8_str(s_sht_back->buf, s_sht_back->bxsize, 0, 120, COL8_FFFFFF, s);
    sheet_refresh(s_sht_back, 0, 120, 200, 136);
}

void debug_print1(const char *fmt, ...) {
    // 320 x 200
    char s[40];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(s, fmt, ap);
    va_end(ap);
    boxfill8(s_sht_back->buf, s_sht_back->bxsize, COL8_000000, 0, 136, 200, 152);
    put_font8_str(s_sht_back->buf, s_sht_back->bxsize, 0, 136, COL8_FFFFFF, s);
    sheet_refresh(s_sht_back, 0, 136, 200, 152);
}

void debug_print2(const char *fmt, ...) {
    // 320 x 200
    char s[40];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(s, fmt, ap);
    va_end(ap);
    boxfill8(s_sht_back->buf, s_sht_back->bxsize, COL8_000000, 0, 152, 200, 168);
    put_font8_str(s_sht_back->buf, s_sht_back->bxsize, 0, 152, COL8_FFFFFF, s);
    sheet_refresh(s_sht_back, 0, 152, 200, 168);
}
