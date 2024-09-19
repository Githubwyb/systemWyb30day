#include "bootpack.h"

#include <linux/compiler.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>

#include "asmfunc.h"
#include "fonts.h"
#include "graphic.h"
#include "log.h"

void make_window8(unsigned char *buf, int xsize, int ysize, char *title);

static struct SHEET *s_sht_back = NULL;

void HariMain(void) {
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    char s[40];
    FIFO32Type fifo;
    INIT_KFIFO(fifo);

    LOG_INFO("HariMain start");
    // 初始化段和中断
    init_gdtidt();
    init_pic();
    io_sti();  // IDT和PIC初始化完成后，解除中断禁止
    init_pit();
    io_out8(PIC0_IMR, 0xf8);  // 11111000 允许PIC1、键盘、定时器的中断
    io_out8(PIC1_IMR, 0xef);  // 11101111 允许鼠标的中断

    LOG_INFO("init gdtidt done");

    // 处理定时器
    struct TIMER timer, timer2, timer3;
    timer_init(&timer, &fifo, 10);
    timer_settime(&timer, jiffies + msecs_to_jiffies(10 * MSEC_PER_SEC));
    timer_init(&timer2, &fifo, 3);
    timer_settime(&timer2, jiffies + msecs_to_jiffies(3 * MSEC_PER_SEC));
    timer_init(&timer3, &fifo, 1);
    timer_settime(&timer3, jiffies + msecs_to_jiffies(500));

    LOG_INFO("create timer done");

    // 键盘鼠标
    init_keyboard(&fifo, 256);
    enable_mouse(&fifo, 512);

    LOG_INFO("init keyboard and mouse done");

    init_palette();

    LOG_INFO("init palette done, screen: size %dx%d, vram 0x%x, vmode %d",
             binfo->scrnx,
             binfo->scrny,
             binfo->vram,
             binfo->vmode);

    // 处理内存
    unsigned long start = jiffies;
    uint32_t memtotal = memtest(0x00400000, 0xbfffffff);
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000);
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    LOG_INFO("init memory done, cost %lums, memory %dMB, free: %dKB",
             jiffies_to_msecs(jiffies - start),
             memtotal / 1024 / 1024,
             memman_total(memman) / 1024);

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
    put_font8_str_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_009999, s);
    sprintf(s, "memory %dMB    free: %dKB", memtotal / 1024 / 1024, memman_total(memman) / 1024);
    put_font8_str_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_009999, s);

    LOG_INFO("init windows done, memory %dMB, free: %dKB", memtotal / 1024 / 1024, memman_total(memman) / 1024);

    struct MOUSE_DEC mdec;
    mdec.phase = 0;
    unsigned long count = 0;
    for (;;) {
        ++count;

        io_cli();
        if (kfifo_is_empty(&fifo)) {
            io_sti();
            continue;
        }

        unsigned int i = 0;
        kfifo_get(&fifo, &i);
        io_sti();
        if (256 <= i && i <= 511) {
            sprintf(s, "%02X", i - 256);
            put_font8_str_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_009999, s);
        } else if (512 <= i && i <= 767) {
            if (!mouse_decode(&mdec, i - 512)) {
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
            put_font8_str_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_009999, s);

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
            put_font8_str_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_009999, s);

            // 把光标画上去
            sheet_slide(sht_mouse, mx, my);
        } else {
            switch (i) {
                case 10:
                    put_font8_str_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_009999, "10[sec]");
                    sprintf(s, "%013lu", count);
                    put_font8_str_sht(sht_win, 40, 28, COL8_000000, COL8_CCCCCC, s);
                    LOG_INFO("count: %lu", count);
                    break;
                case 3:
                    put_font8_str_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_009999, "3[sec]");
                    count = 0;  // 开始测试
                    break;
                default:
                    // 模拟光标
                    if (i != 0) {
                        timer_init(&timer3, &fifo, 0);
                        boxfill8(buf_back, binfo->scrnx, COL8_FFFFFF, 8, 96, 15, 111);
                    } else {
                        timer_init(&timer3, &fifo, 1);
                        boxfill8(buf_back, binfo->scrnx, COL8_009999, 8, 96, 15, 111);
                    }
                    timer_settime(&timer3, jiffies + msecs_to_jiffies(500));
                    sheet_refresh(sht_back, 8, 96, 16, 112);
                    break;
            }
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
