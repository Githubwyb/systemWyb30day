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

struct MOUSE_DEC {
    unsigned char buf[3];  // 记录数据
    uint8_t phase;         // 记录状态
    int x, y, btn;
};

/**
 * @brief 解码data，如果解码完成返回true
 *
 * @param mdec
 * @param data
 * @return bool true和false
 */
static bool mouse_decode(struct MOUSE_DEC *mdec, uint8_t dat) {
    switch (mdec->phase) {
        case 0:
            // 等待0xfa
            if (dat == 0xfa) {
                mdec->phase = 1;
            }
            break;

        case 1:
            // 做一层过滤
            if ((dat & 0xc8) == 0x08) {
                mdec->buf[0] = dat;
                mdec->phase = 2;
            }
            break;
        case 2:
            mdec->buf[1] = dat;
            mdec->phase = 3;
            break;
        case 3:
            mdec->buf[2] = dat;
            mdec->phase = 1;

            // 计算x和y还有btn
            mdec->btn = mdec->buf[0] & 0x07;
            mdec->x = mdec->buf[1];
            mdec->y = mdec->buf[2];
            if ((mdec->buf[0] & 0x10) != 0) {
                mdec->x |= 0xffffff00;
            }
            if ((mdec->buf[0] & 0x20) != 0) {
                mdec->y |= 0xffffff00;
            }
            mdec->y = -mdec->y; /* y和画面方向相反 */
            return true;

        default:
            break;
    }
    return false;
}

void HariMain(void) {
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    struct MOUSE_DEC mdec;

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
    mdec.phase = 0;

    for (;;) {
        io_cli();
        if (kfifo_is_empty(&g_keybuf) && kfifo_is_empty(&g_mouseBuf)) {
            io_stihlt();
        } else {
            unsigned char i;
            char s[20];
            if (!kfifo_is_empty(&g_keybuf)) {
                kfifo_get(&g_keybuf, &i);
                io_sti();
                sprintf(s, "%02X", i);
                boxfill8(binfo->vram, binfo->scrnx, COL8_009999, 0, 16, 15, 31);
                put_font8_str(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
            } else if (!kfifo_is_empty(&g_mouseBuf)) {
                kfifo_get(&g_mouseBuf, &i);
                io_sti();

                if (!mouse_decode(&mdec, i)) {
                    continue;
                }

                // 鼠标数据解读完成，显示画面
                sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
                if ((mdec.btn & 0x01) != 0) {
                    s[1] = 'L';
                }
                if ((mdec.btn & 0x02) != 0) {
                    s[3] = 'R';
                }
                if ((mdec.btn & 0x04) != 0) {
                    s[2] = 'C';
                }
                // 打印鼠标数据
                boxfill8(binfo->vram, binfo->scrnx, COL8_009999, 32, 16, 32 + 20 * 8 - 1, 31);
                put_font8_str(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);

                // 移动光标
                // 当前光标位置涂成背景色
                boxfill8(binfo->vram, binfo->scrnx, COL8_009999, mx, my, mx + 15, my + 15);
                mx += mdec.x;
                my += mdec.y;
                // 限定边界
                if (mx < 0) {
                    mx = 0;
                }
                if (my < 0) {
                    my = 0;
                }
                if (mx > binfo->scrnx - 16) {
                    mx = binfo->scrnx - 16;
                }
                if (my > binfo->scrny - 16) {
                    my = binfo->scrny - 16;
                }
                sprintf(s, "(%3d, %3d)", mx, my);
                boxfill8(binfo->vram, binfo->scrnx, COL8_009999, 0, 0, 79, 15);
                put_font8_str(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
                // 把光标画上去
                putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
            }
        }
    }
}

void debug_print(const char *s) {
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    // 320 x 200
    boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 100, 200, 150);
    put_font8_str(binfo->vram, binfo->scrnx, 0, 100, COL8_FFFFFF, s);
}
