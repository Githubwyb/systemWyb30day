#ifndef __FONTS_H__
#define __FONTS_H__

static u8 font_A[16] = {0x00, 0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
                          0x24, 0x7e, 0x42, 0x42, 0x42, 0xe7, 0x00, 0x00};

/**
 * @brief 输入一个字符
 *
 * @param vram 显示内存起始位置
 * @param xsize 屏幕的宽度，因为要换行显示，需要知道宽度
 * @param x 位置x
 * @param y 位置y
 * @param c 字体颜色
 * @param font 字符矩阵
 */
static void put_font8(u8 *vram, int xsize, int x, int y, u8 c, u8 *font) {
    int i, j;
    for (i = 0; i < 16; ++i) {
        // 找到对应行的最左边
        u8 *p = vram + (y + i) * xsize + x;
        u8 tmp = font[i];
        for (j = 0; j < 8; ++j) {
            if ((tmp & (0x80 >> j)) > 0) {
                p[j] = c;
            }
        }
    }
}

#endif
