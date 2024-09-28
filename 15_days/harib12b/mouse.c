#include <linux/stddef.h>

#include "bootpack.h"
#include "asmfunc.h"

#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

static FIFO32Type *s_mousefifo;
static int s_mousedata0;

/**
 * @brief 使能鼠标
 *
 */
void enable_mouse(FIFO32Type *fifo, int data0) {
    // 设置fifo缓冲区
    s_mousefifo = fifo;
    s_mousedata0 = data0;
    // 设置鼠标
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
    // 如果成功，就会发送ACK(0xfa)
}

void inthandler2c(int *esp) {
    io_out8(PIC1_OCW2, 0x64);  // 通知PIC1，IRQ-12处理完成
    io_out8(PIC0_OCW2, 0x62);  // 通知PIC0，IRQ-02处理完成

    unsigned int data = io_in8(PORT_KEYDAT);
    kfifo_put(s_mousefifo, data + s_mousedata0);
}

bool mouse_decode(struct MOUSE_DEC *mdec, u8 dat) {
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
