#include "bootpack.h"

#include <asm/desc_defs.h>
#include <linux/compiler.h>
#include <linux/input-event-codes.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/processor.h>

#include "asmfunc.h"
#include "fonts.h"
#include "graphic.h"
#include "log.h"

static void make_window8(unsigned char *buf, int xsize, int ysize, char *title);
static void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);

static struct SHEET *s_sht_back = NULL;

static inline void farjmp(u16 seg, u16 offset) {
    __asm__ __volatile__("ljmp *%0\n"
                         :
                         : "m"((struct {
                             unsigned int off;
                             unsigned short sel;
                         }){offset, seg}));
}

static void task_b_main(void) {
    LOG_INFO("task_b_main start");

    FIFO32Type fifo;
    INIT_KFIFO(fifo);
    struct TIMER timer;
    timer_init(&timer, &fifo, 1);
    timer_settime(&timer, jiffies + msecs_to_jiffies(20));

    int count = 0;
    char s[11];
    for (;;) {
        ++count;

        io_cli();
        if (kfifo_is_empty(&fifo)) {
            io_stihlt();
            continue;
        }

        int data = 0;
        kfifo_get(&fifo, &data);
        io_sti();
        if (data == 1) {
            sprintf(s, "%10d", count);
            put_font8_str_sht(s_sht_back, 0, 144, COL8_FFFFFF, COL8_009999, s);
            farjmp(1 * 8, 0x0000);
            timer_settime(&timer, jiffies + msecs_to_jiffies(20));
        }
    }
}

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
    struct TIMER timer, timer2, timer3, timer_ts;
    timer_init(&timer, &fifo, 10);
    timer_settime(&timer, jiffies + msecs_to_jiffies(10 * MSEC_PER_SEC));
    timer_init(&timer2, &fifo, 3);
    timer_settime(&timer2, jiffies + msecs_to_jiffies(3 * MSEC_PER_SEC));
    timer_init(&timer3, &fifo, 1);
    timer_settime(&timer3, jiffies + msecs_to_jiffies(500));
    timer_init(&timer_ts, &fifo, 2);
    timer_settime(&timer_ts, jiffies + msecs_to_jiffies(20));

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

    /********** 任务切换 **********/
    struct desc_struct *gdt = (struct desc_struct *)ADR_GDT;
    struct tss_struct tss_a, tss_b;
    // 申请栈内存，由于栈指针是从高地址向低地址移动，所以esp栈顶指针要设置成最高地址
    int task_b_esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;

    tss_a.x86_tss.ldt = 0;
    tss_b.x86_tss.ldt = 0;

    // 入口地址，由于内核使用的是段内相对地址，我们定义text.first在0x00280000
    // 所以函数需要减去这个地址才得到真正的段内偏移
    tss_b.x86_tss.ip = (int)&task_b_main - ADR_BOTPAK;
    tss_b.x86_tss.flags = 0x00000202;
    tss_b.x86_tss.ax = 0;
    tss_b.x86_tss.cx = 0;
    tss_b.x86_tss.dx = 0;
    tss_b.x86_tss.bx = 0;
    tss_b.x86_tss.sp = task_b_esp;
    tss_b.x86_tss.bp = 0;
    tss_b.x86_tss.si = 0;
    tss_b.x86_tss.di = 0;
    tss_b.x86_tss.es = __KERNEL_DS;
    tss_b.x86_tss.cs = __KERNEL_CS;
    tss_b.x86_tss.ss = __KERNEL_DS;
    tss_b.x86_tss.ds = __KERNEL_DS;
    tss_b.x86_tss.fs = __KERNEL_DS;
    tss_b.x86_tss.gs = __KERNEL_DS;

    tss_desc tss_a_desc;
    tss_desc tss_b_desc;
    set_tssldt_descriptor(&tss_a_desc, (unsigned long)&tss_a.x86_tss, DESC_TSS, __KERNEL_TSS_LIMIT);
    set_tssldt_descriptor(&tss_b_desc, (unsigned long)&tss_b.x86_tss, DESC_TSS, __KERNEL_TSS_LIMIT);
    write_gdt_entry(gdt, 1, &tss_a_desc, DESC_TSS);
    write_gdt_entry(gdt, 2, &tss_b_desc, DESC_TSS);
    load_tr_desc(1 * 8);

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
    make_window8(buf_win, 160, 52, "window");
    sheet_setbuf(sht_win, buf_win, 160, 52, -1);  // 没有透明色
    make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
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
    int cursor_x = 8;            // 光标位置
    int cursor_c = COL8_FFFFFF;  // 光标颜色，白色
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
        if (i == 2) {
            farjmp(2 * 8, 0x0000);
            timer_settime(&timer_ts, jiffies + msecs_to_jiffies(20));
        } else if (256 <= i && i <= 511) {
            // 处理键盘数据
            i -= 256;
            static char keytable[0x54] = {
                [KEY_0] = '0',          [KEY_1] = '1',          [KEY_2] = '2',         [KEY_3] = '3',
                [KEY_4] = '4',          [KEY_5] = '5',          [KEY_6] = '6',         [KEY_7] = '7',
                [KEY_8] = '8',          [KEY_9] = '9',          [KEY_A] = 'a',         [KEY_B] = 'b',
                [KEY_C] = 'c',          [KEY_D] = 'd',          [KEY_E] = 'e',         [KEY_F] = 'f',
                [KEY_G] = 'g',          [KEY_H] = 'h',          [KEY_I] = 'i',         [KEY_J] = 'j',
                [KEY_K] = 'k',          [KEY_L] = 'l',          [KEY_M] = 'm',         [KEY_N] = 'n',
                [KEY_O] = 'o',          [KEY_P] = 'p',          [KEY_Q] = 'q',         [KEY_R] = 'r',
                [KEY_S] = 's',          [KEY_T] = 't',          [KEY_U] = 'u',         [KEY_V] = 'v',
                [KEY_W] = 'w',          [KEY_X] = 'x',          [KEY_Y] = 'y',         [KEY_Z] = 'z',
                [KEY_SPACE] = ' ',      [KEY_MINUS] = '-',      [KEY_EQUAL] = '=',     [KEY_LEFTBRACE] = '[',
                [KEY_RIGHTBRACE] = ']', [KEY_BACKSLASH] = '\\', [KEY_SEMICOLON] = ';', [KEY_APOSTROPHE] = '\'',
                [KEY_GRAVE] = '`',      [KEY_COMMA] = ',',      [KEY_DOT] = '.',       [KEY_SLASH] = '/'};
            sprintf(s, "%02X", i);
            put_font8_str_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_009999, s);
            if (i < 0x54) {
                if (keytable[i] != 0 && cursor_x < 144) {
                    // 可见字符且光标没有超出范围
                    // 显示一个字符且移动一次光标
                    s[0] = keytable[i];
                    s[1] = 0;
                    put_font8_str_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s);
                    cursor_x += 8;
                }
            }
            if (i == KEY_BACKSPACE && cursor_x > 8) {
                // 删除键且有字符
                // 将光标所在位置写一个空格用于抹除光标，然后移动光标
                put_font8_str_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ");
                cursor_x -= 8;
            }
            // 显示光标，仅刷新光标所在位置，写入是在后面显示光标，删除是在前一个字符位置显示光标，也就是删掉了
            boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
            sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
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

            // 把鼠标画上去
            sheet_slide(sht_mouse, mx, my);

            if ((mdec.btn & 0x01) != 0) {
                // 按下左键，移动sht_win
                sheet_slide(sht_win, mx - 80, my - 8);
            }
        } else {
            switch (i) {
                case 10:
                    put_font8_str_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_009999, "10[sec]");
                    LOG_INFO("count: %lu", count);
                    break;
                case 3:
                    put_font8_str_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_009999, "3[sec]");
                    count = 0;  // 开始测试
                    break;
                default:
                    // 给光标用的定时器
                    if (i != 0) {
                        timer_init(&timer3, &fifo, 0);
                        cursor_c = COL8_000000;  // 显示黑色
                    } else {
                        timer_init(&timer3, &fifo, 1);
                        cursor_c = COL8_FFFFFF;  // 显示白色
                    }
                    timer_settime(&timer3, jiffies + msecs_to_jiffies(500));
                    boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
                    sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
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

void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c) {
    int x1 = x0 + sx, y1 = y0 + sy;
    boxfill8(sht->buf, sht->bxsize, COL8_999999, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
    boxfill8(sht->buf, sht->bxsize, COL8_999999, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
    boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
    boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
    boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1, y0 - 2);
    boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1);
    boxfill8(sht->buf, sht->bxsize, COL8_CCCCCC, x0 - 2, y1 + 1, x1, y1 + 1);
    boxfill8(sht->buf, sht->bxsize, COL8_CCCCCC, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
    boxfill8(sht->buf, sht->bxsize, c, x0 - 1, y0 - 1, x1, y1);
}
