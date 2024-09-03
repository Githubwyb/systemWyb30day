#include <linux/kernel.h>

#include "asmfunc.h"
#include "bootpack.h"
#include "fonts.h"
#include "graphic.h"

void init_pic() {
    io_out8(PIC0_IMR, 0xff);  // 禁止所有中断
    io_out8(PIC1_IMR, 0xff);  // 禁止所有中断

    io_out8(PIC0_ICW1, 0x11);    // 边沿触发模式
    io_out8(PIC0_ICW2, 0x20);    // IRQ0-7由INT20-27接收
    io_out8(PIC0_ICW3, 1 << 2);  // PIC1由IRQ2连接
    io_out8(PIC0_ICW4, 0x01);    // 无缓冲区模式

    io_out8(PIC1_ICW1, 0x11);    // 边沿触发模式
    io_out8(PIC1_ICW2, 0x28);    // IRQ8-15由INT28-2f接收
    io_out8(PIC1_ICW3, 1 << 1);  // PIC1由IRQ2连接
    io_out8(PIC1_ICW4, 0x01);    // 无缓冲区模式

    io_out8(PIC0_IMR, 0xfb);  // 11111011 PIC1以外全部禁止
    io_out8(PIC1_IMR, 0xff);  // 禁止所有中断
}

#define PORT_KEYDAT 0x0060

struct KEYBUF g_keybuf;
/**
 * @brief 处理PS/2键盘的中断
 *
 * @param esp
 */
void inthandler21(int *esp) {
    io_out8(PIC0_OCW2, 0x61);  // 通知PIC的IRQ_01已经处理完毕

    unsigned char data = io_in8(PORT_KEYDAT);  // 读取键盘输入的值
    if (g_keybuf.len < 32) {
        g_keybuf.data[g_keybuf.next_w] = data;
        g_keybuf.len++;
        g_keybuf.next_w = g_keybuf.next_w == 31 ? 0 : g_keybuf.next_w + 1;
    }
}

/**
 * @brief 处理PS/2鼠标的中断
 *
 * @param esp
 */
void inthandler2c(int *esp) {
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;

    boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
    put_font8_str(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 2C (IRQ-12): PS/2 mouse");
    for (;;) {
        io_hlt();
    }
}

/**
 * @brief 处理27中断，athlon64x2机等芯片组的原因，在PIC初始化时会发生一次中断
 *        此中断什么都不用做就好，一般是由于PIC初始化的电噪声引起的
 *
 * @param esp
 */
void inthandler27(int *esp) {
    io_out8(PIC0_OCW2, 0x67);  // 向PIC通知IRQ-07处理完成
    return;
}
