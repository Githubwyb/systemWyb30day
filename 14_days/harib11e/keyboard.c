#include <linux/stddef.h>

#include "asmfunc.h"
#include "bootpack.h"

#define PORT_KEYDAT 0x0060
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47

static FIFO32Type *s_keyfifo;
static int s_keydata0;

/**
 * @brief 键盘控制器的初始化
 *
 */
void init_keyboard(FIFO32Type *fifo, int data0) {
    // 初始化FIFO
    s_keyfifo = fifo;
    s_keydata0 = data0;
    // 初始化键盘
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
}

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

void inthandler21(int *esp) {
    io_out8(PIC0_OCW2, 0x61);  // 通知PIC的IRQ_01已经处理完毕

    unsigned int data = io_in8(PORT_KEYDAT);  // 读取键盘输入的值
    kfifo_put(s_keyfifo, data + s_keydata0);
}
