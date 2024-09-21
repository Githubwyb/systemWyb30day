#include <linux/types.h>

#include "bootpack.h"

static void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0);

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize) {
    struct SHTCTL *ctl = NULL;
    int i;
    ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof(struct SHTCTL));
    if (ctl == NULL) {
        goto err;
    }
    ctl->map = (unsigned char *)memman_alloc_4k(memman, xsize * ysize);
    if (ctl->map == NULL) {
        goto err;
    }
    ctl->vram = vram;
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    ctl->top = 0;  // 没有sheet
    for (i = 0; i < MAX_SHEETS; i++) {
        ctl->sheets0[i].flags = 0;  // 全部标记未使用
        ctl->sheets0[i].ctl = ctl;
    }
    return ctl;
err:
    if (ctl->map != NULL) {
        memman_free_4k(memman, (int)ctl->map, xsize * ysize);
        ctl->map = NULL;
    }
    if (ctl != NULL) {
        memman_free_4k(memman, (int)ctl, sizeof(struct SHTCTL));
        ctl = NULL;
    }
    return NULL;
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

void sheet_updown(struct SHEET *sht, int height) {
    struct SHTCTL *ctl = sht->ctl;
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
            // 降低了，别的在它上面，此层不用画了，直接刷其上面的
            sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, sht->height + 1);
            sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, sht->height + 1, old);
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
            // 隐藏要从0开始刷，不知道后面有几层
            sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
            sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0, old - 1);
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
        sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, sht->height);
        sheet_refreshsub(
            ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, sht->height, sht->height);
    }
}

// 只刷新一个sheet的特定区域，用于某个图层中一部分区域改变，不刷新整个图层
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1) {
    struct SHTCTL *ctl = sht->ctl;
    if (sht->height >= 0) {
        // sht没有隐藏才要刷新
        // 由于refresh没有改变sheet的上下关系，也没有动窗口，不用更新map
        // 同时由于map的存在，更新一层即可，因为map指明了要用哪一层，此层要用才会更新
        sheet_refreshsub(ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1, sht->height, sht->height);
    }
}

// 刷新map，修改对应范围取哪个图层的像素点
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0) {
    int h, bx, by, vx, vy, bx0, by0, bx1, by1;
    unsigned char *buf, *vram = ctl->vram;
    unsigned char c;
    struct SHEET *sht;
    unsigned char *map = ctl->map;
    unsigned char sid;  // 取对应图层在所有sheet中的索引为sid

    vx0 = vx0 < 0 ? 0 : vx0;
    vy0 = vy0 < 0 ? 0 : vy0;
    vx1 = vx1 > ctl->xsize ? ctl->xsize : vx1;
    vy1 = vy1 > ctl->ysize ? ctl->ysize : vy1;
    for (h = h0; h <= ctl->top; ++h) {
        sht = ctl->sheets[h];
        sid = sht - ctl->sheets0;
        // 针对每个sheet计算在不在刷新范围内，不在则跳过
        // 先计算交叉区域
        bx0 = vx0 - sht->vx0;  // vx0在sht中buf的相对位置
        if (bx0 < 0) bx0 = 0;  // 范围限制
        by0 = vy0 - sht->vy0;
        if (by0 < 0) by0 = 0;

        bx1 = vx1 - sht->vx0;
        if (bx1 > sht->bxsize) bx1 = sht->bxsize;
        by1 = vy1 - sht->vy0;
        if (by1 > sht->bysize) by1 = sht->bysize;

        buf = sht->buf;
        for (by = by0; by < by1; ++by) {
            vy = sht->vy0 + by;
            for (bx = bx0; bx < bx1; ++bx) {
                vx = sht->vx0 + bx;
                c = buf[by * sht->bxsize + bx];
                if (c != sht->col_inv) {
                    // 如果颜色不是透明色，则需要赋值
                    map[vy * ctl->xsize + vx] = sid;
                }
            }
        }
    }
}

// 对整体刷新变动的位置，h0表示从哪个图层开始刷新
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1) {
    int h, bx, by, vx, vy, bx0, by0, bx1, by1;
    unsigned char *buf, *vram = ctl->vram;
    unsigned char c;
    struct SHEET *sht;
    unsigned char *map = ctl->map;
    unsigned char sid;  // 取对应图层在所有sheet中的索引为sid

    vx0 = vx0 < 0 ? 0 : vx0;
    vy0 = vy0 < 0 ? 0 : vy0;
    vx1 = vx1 > ctl->xsize ? ctl->xsize : vx1;
    vy1 = vy1 > ctl->ysize ? ctl->ysize : vy1;
    for (h = h0; h <= ctl->top && h <= h1; ++h) {
        sht = ctl->sheets[h];
        sid = sht - ctl->sheets0;
        // 针对每个sheet计算在不在刷新范围内，不在则跳过
        // 先计算交叉区域
        bx0 = vx0 - sht->vx0;  // vx0在sht中buf的相对位置
        if (bx0 < 0) bx0 = 0;  // 范围限制
        by0 = vy0 - sht->vy0;
        if (by0 < 0) by0 = 0;

        bx1 = vx1 - sht->vx0;
        if (bx1 > sht->bxsize) bx1 = sht->bxsize;
        by1 = vy1 - sht->vy0;
        if (by1 > sht->bysize) by1 = sht->bysize;

        buf = sht->buf;
        for (by = by0; by < by1; ++by) {
            vy = sht->vy0 + by;
            for (bx = bx0; bx < bx1; ++bx) {
                vx = sht->vx0 + bx;
                // map中已经处理过透明层的问题了，这里判断属于此图层就赋值
                if (map[vy * ctl->xsize + vx] == sid) {
                    vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
                }
            }
        }
    }
}

void sheet_slide(struct SHEET *sht, int vx0, int vy0) {
    struct SHTCTL *ctl = sht->ctl;
    int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
    sht->vx0 = vx0;
    sht->vy0 = vy0;
    if (sht->height >= 0) {
        // sht没有隐藏才要刷新
        // 老的sheet区域不知道哪个在最底，从最底层开始刷新，因为滑走了，不用更新到这一层
        sheet_refreshmap(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
        sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0, sht->height - 1);
        // 新的直接从新的开始写即可
        sheet_refreshmap(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
        sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height, sht->height);
    }
}

void sheet_free(struct SHEET *sht) {
    struct SHTCTL *ctl = sht->ctl;
    if (sht->height >= 0) {
        sheet_updown(sht, -1);
    }
    sht->flags = 0;
}