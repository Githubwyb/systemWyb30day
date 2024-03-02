/**
 * @file sheet.c
 * @author 王钰博18433 (18433@sangfor.com)
 * @brief 图层相关代码
 * @version 0.1
 * @date 2024-02-27
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <linux/types.h>
#include <linux/stddef.h>

#include "bootpack.h"

#define SHEET_USE 1

struct SHEET {
    u8 *buf;             // 图层画面内容地址
    int bxsize, bysize;  // 图层整体大小
    int vx0, vy0;        // 图层在画面位置坐标
    int col_inv;         // 透明色色号
    int height;          // 高度
    int flags;           // 设定信息，0为未使用
};

#define MAX_SHEETS 256

struct SHTCTL {
    u8 *vram;                          // vram地址
    int xsize, ysize;                  // vram大小
    int top;                           // 最上面图层高度
    struct SHEET *sheets[MAX_SHEETS];  // 按照高度进行排列的指针数组
    struct SHEET sheets0[MAX_SHEETS];  // 存放所有图层
};

struct SHTCTL *shtctl_init(struct MEMMAN *memman, u8 *vram, int xsize, int ysize) {
    struct SHTCTL *ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof(struct SHTCTL));
    if (!ctl) return NULL;
    ctl->vram = vram;
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    ctl->top = -1;
    for (int i = 0; i < MAX_SHEETS; i++) {
        ctl->sheets[i] = NULL;
        ctl->sheets0[i].flags = 0;
    }
    return ctl;
}

struct SHEET *sheet_add(struct SHTCTL *ctl) {
    for (int i = 0; i < MAX_SHEETS; i++) {
        if (ctl->sheets0[i].flags != SHEET_USE) {
            struct SHEET *sht = &ctl->sheets0[i];
            sht->flags = SHEET_USE;  // 在使用
            sht->height = -1;        // 隐藏
            return sht;
        }
    }
    return NULL;
}
