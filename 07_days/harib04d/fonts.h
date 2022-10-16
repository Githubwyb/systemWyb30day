#ifndef __FONTS_H__
#define __FONTS_H__

// 由hankaku.txt编译而来
extern char hankaku[4096];

/**
 * @brief 显示一个字符
 *
 * @param vram 显示内存起始位置
 * @param xsize 屏幕的宽度，因为要换行显示，需要知道宽度
 * @param x 位置x
 * @param y 位置y
 * @param color 字体颜色
 * @param c 字符
 */
static void put_font8(char *vram, int xsize, int x, int y, char color, char c) {
    int i, j;
    char *font8 = hankaku + c * 16;
    for (i = 0; i < 16; ++i) {
        // 找到对应行的最左边
        char *p = vram + (y + i) * xsize + x;
        char tmp = font8[i];
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
static void put_font8_str(char *vram, int xsize, int x, int y, char color, unsigned char *s) {
    int i;
    for (i = 0; s[i] != '\0'; ++i) {
        put_font8(vram, xsize, x + 8 * i, y, color, s[i]);
    }
}

#endif
