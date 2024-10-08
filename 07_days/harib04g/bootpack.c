#include "bootpack.h"

#include <linux/compiler.h>
#include <linux/kernel.h>

#include "asmfunc.h"
#include "fonts.h"
#include "graphic.h"

#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47  // 鼠标模式

/**
 * @brief 等待键盘控制器可以发送数据
 *
 */
void wait_KBC_sendready() {
    for (;;) {
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
            break;
        }
    }
}

/**
 * @brief 键盘控制器的初始化
 *
 */
void init_keyboard() {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    // 设置模式使用鼠标
    io_out8(PORT_KEYDAT, KBC_MODE);
    return;
}

#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

/**
 * @brief 使能鼠标
 *
 */
void enable_mouse() {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
    // 如果成功，就会发送ACK(0xfa)
}

void HariMain(void) {
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;

    // 初始化段和中断
    init_gdtidt();
    init_pic();
    io_sti();  // IDT和PIC初始化完成后，解除中断禁止

    init_palette();
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

    unsigned char mcursor[256];
    int mx = (binfo->scrnx - 16) / 2;
    int my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_009999);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

    io_out8(PIC0_IMR, 0xf9);  // 11111001 允许PIC1和键盘的中断
    io_out8(PIC1_IMR, 0xef);  // 11101111 允许鼠标的中断

    init_keyboard();
    enable_mouse();

    for (;;) {
        io_cli();
        if (kfifo_is_empty(&g_keybuf) && kfifo_is_empty(&g_mouseBuf)) {
            io_stihlt();
        } else {
            unsigned char i;
            char s[4];
            if (!kfifo_is_empty(&g_keybuf)) {
                kfifo_get(&g_keybuf, &i);
                io_sti();
                sprintf(s, "%02X", i);
                boxfill8(binfo->vram, binfo->scrnx, COL8_009999, 0, 16, 15, 31);
                put_font8_str(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
            } else if (!kfifo_is_empty(&g_mouseBuf)) {
                kfifo_get(&g_mouseBuf, &i);
                io_sti();
                sprintf(s, "%02X", i);
                boxfill8(binfo->vram, binfo->scrnx, COL8_009999, 32, 16, 47, 31);
                put_font8_str(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
            }
        }
    }
}
