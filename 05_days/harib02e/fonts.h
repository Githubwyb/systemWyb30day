#ifndef __FONTS_H__
#define __FONTS_H__

// 由hankaku.txt编译而来
extern char hankaku[4096];

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

#endif
