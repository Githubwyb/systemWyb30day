#include <linux/stddef.h>

#include "bootpack.h"
#include "naskfunc.h"

#define PORT_KEYDAT 0x0060
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47

/**
 * @brief 键盘控制器的初始化
 *
 */
void init_keyboard() {
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

KeyBufType g_keybuf = (typeof(g_keybuf)){{{
    .in = 0,
    .out = 0,
    .mask = __is_kfifo_ptr(&(g_keybuf)) ? 0 : ARRAY_SIZE((g_keybuf).buf) - 1,
    .esize = sizeof(*(g_keybuf).buf),
    .data = __is_kfifo_ptr(&(g_keybuf)) ? NULL : (g_keybuf).buf,
}}};

void inthandler21(int *esp) {
    io_out8(PIC0_OCW2, 0x61);  // 通知PIC的IRQ_01已经处理完毕

    unsigned char data = io_in8(PORT_KEYDAT);  // 读取键盘输入的值
    kfifo_put(&g_keybuf, data);
}
