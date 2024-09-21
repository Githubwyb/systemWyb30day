#ifndef __FONTS_H__
#define __FONTS_H__

#include <linux/string.h>
#include <linux/types.h>

#include "bootpack.h"
#include "graphic.h"

extern u8 hankaku[2048];

/**
 * @brief 输入一个字符
 *
 * @param vram 显示内存起始位置
 * @param xsize 屏幕的宽度，因为要换行显示，需要知道宽度
 * @param x 位置x
 * @param y 位置y
 * @param color 字体颜色
 * @param c 字符
 */
static void put_font8(u8 *vram, int xsize, int x, int y, char color, const char c) {
    int i, j;
    u8 *font8 = hankaku + c * 16;
    for (i = 0; i < 16; ++i) {
        // 找到对应行的最左边
        u8 *p = vram + (y + i) * xsize + x;
        u8 tmp = font8[i];
        for (j = 0; j < 8; ++j) {
            if ((tmp & (0x80 >> j)) > 0) {
                p[j] = color;
            }
        }
    }
}

/**
 * @brief 显示一个字符串
 *
 * @param vram 显示内存起始位置
 * @param xsize 屏幕的宽度，因为要换行显示，需要知道宽度
 * @param x 位置x
 * @param y 位置y
 * @param color 字体颜色
 * @param s 字符串
 */
static void put_font8_str(u8 *vram, int xsize, int x, int y, char color, const char *s) {
    int i;
    for (i = 0; s[i] != '\0'; ++i) {
        put_font8(vram, xsize, x + 8 * i, y, color, s[i]);
    }
}

/**
 * @brief 在某个图层显示一个字符串
 *
 * @param sht 图层地址
 * @param x
 * @param y
 * @param c 字体颜色
 * @param b 背景颜色
 * @param s
 */
static void put_font8_str_sht(struct SHEET *sht, int x, int y, int c, int b, const char *s) {
    int x1 = x + 8 * strlen(s) - 1;
    int y1 = y + 15;
    boxfill8(sht->buf, sht->bxsize, b, x, y, x1, y1);
    put_font8_str(sht->buf, sht->bxsize, x, y, c, s);
    sheet_refresh(sht, x, y, x1 + 1, y1 + 1);
}

#endif
