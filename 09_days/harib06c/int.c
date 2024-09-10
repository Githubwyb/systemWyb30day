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
