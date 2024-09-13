#ifndef __GRAPHIC_H__
#define __GRAPHIC_H__

#include <linux/types.h>

#define COL8_000000 215
#define COL8_FF0000 35
#define COL8_00FF00 185
#define COL8_FFFF00 5
#define COL8_0000FF 210
#define COL8_FF00FF 30
#define COL8_00FFFF 180
#define COL8_FFFFFF 0
#define COL8_CCCCCC 79
#define COL8_990000 179
#define COL8_009900 197
#define COL8_999900 161
#define COL8_000099 212
#define COL8_990099 176
#define COL8_009999 194
#define COL8_999999 158

void init_palette();
void init_mouse_cursor8(u8 *mouse, u8 bc);
void init_screen(u8 *vram, int xsize, int ysize);
void putblock8_8(u8 *vram, int vxsize, int pxsize, int pysize, int px0, int py0, u8 *buf, int bxsize);
/**
 * @brief 显示器对一块矩形区域填充
 *
 * @param vram 显示内存起始地址
 * @param xsize 显示的宽度
 * @param c 颜色
 * @param x0
 * @param y0
 * @param x1
 * @param y1
 */
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);

#endif  // __GRAPHIC_H__
