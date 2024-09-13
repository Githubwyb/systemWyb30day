#include <linux/types.h>

#include "bootpack.h"

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize) {
    struct SHTCTL *ctl = NULL;
    int i;
    ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof(struct SHTCTL));
    if (ctl == NULL) {
        goto err;
    }
    ctl->vram = vram;
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    ctl->top = 0;  // 没有sheet
    for (i = 0; i < MAX_SHEETS; i++) {
        ctl->sheets0[i].flags = 0;  // 全部标记未使用
    }
err:
    return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl) {
    struct SHEET *sht;
    int i;
    for (i = 0; i < MAX_SHEETS; i++) {
        if (ctl->sheets0[i].flags != SHEET_USE) {
            sht = &ctl->sheets0[i];
            sht->flags = SHEET_USE;  // 标记被使用了
            sht->height = -1;        // 隐藏
            return sht;
        }
    }
    return NULL;  // 所有sheet都被使用了
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv) {
    sht->buf = buf;
    sht->bxsize = xsize;
    sht->bysize = ysize;
    sht->col_inv = col_inv;
}

void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height) {
    int h, old = sht->height;  // 保存旧的高度

    // 做一下边界处理
    if (height >= ctl->top) {
        height = ctl->top;
    }
    if (height < -1) {
        height = -1;
    }
    if (old == height) return;
    sht->height = height;

    // 对sheet进行排序
    if (old > height) {
        // 如果高度变小，则需要把应该比它高的sheet都往高处移动，并修改高度
        if (height >= 0) {
            // sht不隐藏
            // 0:0#0 1:1#1 2:2#2 3:(3#3) 将3号sheet高度移到1 => 0:0#0 1:(3#1) 2:1#(2) 3:2#(3)
            for (h = old; h > height; --h) {
                ctl->sheets[h] = ctl->sheets[h - 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        } else {
            // sht要隐藏掉
            // 0:0#0 1:1#1 2:(2#2) 3:3#3 将2号sheet高度移到-1 = 0:0#0 1:1#1 2:3#(2) 3:nil
            if (old != -1) {
                for (h = old; h < ctl->top; ++h) {
                    ctl->sheets[h] = ctl->sheets[h + 1];
                    ctl->sheets[h]->height = h;
                }
                --(ctl->top);
            }
        }
    } else {
        if (old >= 0) {
            // 原来的sht没有隐藏
            // 如果高度变大，则需要把应该比它低的sheet都往低处移动，并修改高度
            // 0:0#0 1:1#1 2:(2#2) 3:3#3 将2号sheet高度移到3 => 0:0#0 1:1#1 2:3#(2) 3:2#(3)
            for (h = old; h < height; ++h) {
                ctl->sheets[h] = ctl->sheets[h + 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        } else {
            // 原来的sht是隐藏的，要将应该比它高的sheet都往高处移动，并修改高度
            // 0:0#0 1:1#1 2:2#2 将3号sheet高度移到2 => 0:0#0 1:1#1 2:3#(2) 3:2#(3)
            for (h = ctl->top; h > height; --h) {
                ctl->sheets[h] = ctl->sheets[h - 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
            ++(ctl->top);
        }
    }
    // 图层高度改变，需要刷新，只刷新此图层的区域
    sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize);
}

// 只刷新一个sheet的特定区域，用于某个图层中一部分区域改变，不刷新整个图层
void sheet_refresh(struct SHTCTL *ctl, struct SHEET *sht, int bx0, int by0, int bx1, int by1) {
    if (sht->height >= 0) {
        // sht没有隐藏才要刷新
        sheet_refreshsub(ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1);
    }
}

// 对整体刷新变动的位置
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1) {
    int h, bx, by, vx, vy, bx0, by0, bx1, by1;
    unsigned char *buf, *vram = ctl->vram;
    unsigned char c;
    struct SHEET *sht;
    for (h = 0; h <= ctl->top; ++h) {
        sht = ctl->sheets[h];
        buf = sht->buf;
        for (by = 0; by < sht->bysize; ++by) {
            vy = sht->vy0 + by;
            for (bx = 0; bx < sht->bxsize; ++bx) {
                vx = sht->vx0 + bx;
                if (vx0 <= vx && vx < vx1 && vy0 <= vy && vy < vy1) {
                    // 如果当前点在需要刷新的范围内
                    c = buf[by * sht->bxsize + bx];
                    if (c != sht->col_inv) {
                        // 如果颜色不是透明色，则需要赋值
                        vram[vy * ctl->xsize + vx] = c;
                    }
                }
            }
        }
    }
}

void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0) {
    int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
    sht->vx0 = vx0;
    sht->vy0 = vy0;
    if (sht->height >= 0) {
        // sht没有隐藏才要刷新
        sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize);
        sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize);
    }
}

void sheet_free(struct SHTCTL *ctl, struct SHEET *sht) {
    if (sht->height >= 0) {
        sheet_updown(ctl, sht, -1);
    }
    sht->flags = 0;
}
