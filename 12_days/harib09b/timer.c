
#include <linux/i8253.h>
#include <linux/timex.h>

#include "asmfunc.h"
#include "bootpack.h"

volatile unsigned long jiffies = 0;

void init_pit(void) {
    // 芯片主频为1193180Hz，设置计数器为11932，对应100Hz
    io_out8(PIT_MODE, 0x34);
    io_out8(PIT_CH0, PIT_LATCH & 0xff);
    io_out8(PIT_CH0, PIT_LATCH >> 8);
}

void inthandler20(int *esp) {
    io_out8(PIC0_OCW2, 0x60);  // 通知PIC已经处理完毕
    ++jiffies;
}
