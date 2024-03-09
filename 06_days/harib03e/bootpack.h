#ifndef __BOOTPACK_H__
#define __BOOTPACK_H__

// boot相关信息，内存地址 0x0ff0-0x0fff
struct BOOTINFO {
    char cyls;
    char leds;
    char vmode;     // 颜色数目的信息，颜色的位数
    char reserve;
    short scrnx;    // 分辨率的x
    short scrny;    // 分辨率的y
    u8 *vram;     // 图像缓冲区开始地址
};
#define ADR_BOOTINFO 0x00000ff0

/* dsctbl.c */
void init_gdtidt();

/* int.c */
#define PIC0_ICW1 0x0020
#define PIC0_OCW2 0x0020
#define PIC0_IMR 0x0021
#define PIC0_ICW2 0x0021
#define PIC0_ICW3 0x0021
#define PIC0_ICW4 0x0021
#define PIC1_ICW1 0x00a0
#define PIC1_OCW2 0x00a0
#define PIC1_IMR 0x00a1
#define PIC1_ICW2 0x00a1
#define PIC1_ICW3 0x00a1
#define PIC1_ICW4 0x00a1

void init_pic();

#endif  // __BOOTPACK_H__
