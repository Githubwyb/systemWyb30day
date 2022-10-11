#ifndef __GRAPHIC_H__
#define __GRAPHIC_H__

void init_palette();
void init_mouse_cursor8(char *mouse, char bc);
void init_screen(char *vram, int xsize, int ysize);
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);

#endif  // __GRAPHIC_H__