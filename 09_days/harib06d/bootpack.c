#include "bootpack.h"

#include <linux/compiler.h>
#include <linux/kernel.h>

#include "asmfunc.h"
#include "fonts.h"
#include "graphic.h"
#define MEMMAN_FREES 4096  // 大约32KB
#define MEMMAN_ADDR 0x003c0000

// 可用信息
struct FREEINFO {
    uint32_t addr;
    uint32_t size;
};

// 内存管理
struct MEMMAN {
    int32_t frees;                       // free信息表的占用总数
    int32_t maxfrees;                    // 用于观察可用状况：frees的最大值
    int32_t lostsize;                    // 释放失败的内存的大小总和
    int32_t losts;                       // 释放失败的次数
    struct FREEINFO free[MEMMAN_FREES];  // 所有free信息表
};

void memman_init(struct MEMMAN *man) {
    man->frees = 0;
    man->maxfrees = 0;
    man->lostsize = 0;
    man->losts = 0;
}
/**
 * @brief 获取剩余内存大小
 *
 * @param man
 * @return uint32_t
 */
uint32_t memman_total(struct MEMMAN *man) {
    uint32_t i = 0;
    uint32_t t = 0;
    for (i = 0; i < man->frees; ++i) {
        t += man->free[i].size;
    }
    return t;
}

uint32_t memman_alloc(struct MEMMAN *man, uint32_t size) {
    uint32_t i = 0;
    uint32_t a = 0;
    for (i = 0; i < man->frees; ++i) {
        if (man->free[i].size >= size) {
            // 找到足够大的内存
            a = man->free[i].addr;
            man->free[i].addr += size;
            man->free[i].size -= size;
            if (man->free[i].size == 0) {
                // free变成0，就将free去掉
                for (; i < man->frees; ++i) {
                    man->free[i] = man->free[i + 1];
                }
            }
            return a;
        }
    }
    return 0;
}

int memman_free(struct MEMMAN *man, uint32_t addr, uint32_t size) {
    int32_t i;  // 要存入内存的idx
    int32_t j;
    // 为了便于归纳内存，将free[]按照addr的顺序排列
    // 先决定放哪里
    for (i = 0; i < man->frees; ++i) {
        if (man->free[i].addr > addr) {
            break;
        }
    }
    // free[i-1].addr < addr < free[i].addr
    if (i > 0 && man->free[i - 1].addr + man->free[i - 1].size == addr) {
        // 可以与前面的内存归纳到一起
        man->free[i - 1].size += size;
        if (i < man->frees) {
            if (addr + size == man->free[i].addr) {
                // 后面下一个块也可以归纳到一起
                man->free[i - 1].size += man->free[i].size;
                man->frees--;
                for (; i < man->frees - 1; ++i) {
                    man->free[i] = man->free[i + 1];
                }
            }
        }
        return 0;
    }
    // 不能与前面归纳到一起但是可以和后面归纳到一起
    if (i < man->frees && addr + size == man->free[i].addr) {
        man->free[i].addr = addr;
        man->free[i].size += size;
        return 0;
    }
    // 既不能和前面归纳到一起，也不能和后面归纳到一起，新建一个info
    if (man->frees < MEMMAN_FREES) {
        // 腾个位置
        for (j = man->frees; j > i; --j) {
            man->free[j] = man->free[j - 1];
        }
        ++man->frees;
        if (man->maxfrees < man->frees) {
            man->maxfrees = man->frees;
        }
        man->free[i].addr = addr;
        man->free[i].size = size;
        return 0;
    }
    // 都不行，内存丢弃
    ++man->losts;
    man->lostsize += size;
    return -1;
}

unsigned int memtest(unsigned int start, unsigned int end);

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
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
    // 画鼠标
    unsigned char mcursor[256];
    int mx = (binfo->scrnx - 16) / 2;
    int my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_009999);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

    // 处理内存
    uint32_t memtotal = memtest(0x00400000, 0xbfffffff);
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000);
    memman_free(memman, 0x00400000, memtotal - 0x00400000);
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
                boxfill8(binfo->vram, binfo->scrnx, COL8_009999, 32, 16, 32 + 20 * 8 - 1, 31);
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

#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000

static unsigned int memtest_sub(unsigned int start, unsigned int end) {
    unsigned int i, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
    volatile unsigned int *p;  // 这里加上volatile，防止编译器优化
    // unsigned int *p = 0;
    for (i = start; i <= end; i += 0x1000) {
        p = (unsigned int *)(i + 0xffc);
        old = *p;         /* 记录以前的值 */
        *p = pat0;        /* 尝试写 */
        *p ^= 0xffffffff; /* 反转 */
        if (*p != pat1) { /* 检查反转结果 */
        not_memory:
            *p = old;
            break;
        }
        *p ^= 0xffffffff; /* 再次反转 */
        if (*p != pat0) { /* 检查反转结果 */
            goto not_memory;
        }
        *p = old; /* 恢复原来的值 */
    }
    return i;
}

unsigned int memtest(unsigned int start, unsigned int end) {
    bool before_386;
    unsigned int eflg, cr0, i;

    // 386及之前的cpu，没有AC这个标记位，所以设置之后还是返回0，使用此方式判断是否为386及以前
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT;
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    before_386 = (eflg & EFLAGS_AC_BIT) == 0;
    // 还原AC标记位
    eflg &= ~EFLAGS_AC_BIT;
    io_store_eflags(eflg);

    if (!before_386) {
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE; /* 禁止缓存 */
        store_cr0(cr0);
    }

    i = memtest_sub(start, end);

    if (!before_386) {
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE; /* 高速缓存许可 */
        store_cr0(cr0);
    }

    return i;
}

void debug_print(const char *s) {
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    // 320 x 200
    boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 100, 200, 150);
    put_font8_str(binfo->vram, binfo->scrnx, 0, 100, COL8_FFFFFF, s);
}
