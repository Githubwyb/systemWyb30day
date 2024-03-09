#ifndef __GRAPHIC_H__
#define __GRAPHIC_H__

#include <linux/types.h>

void init_palette();
void init_mouse_cursor8(u8 *mouse, u8 bc);
void init_screen(u8 *vram, int xsize, int ysize);
void putblock8_8(u8 *vram, int vxsize, int pxsize, int pysize, int px0, int py0, u8 *buf, int bxsize);

#endif  // __GRAPHIC_H__
