
#include <linux/i8253.h>
#include <linux/timex.h>

#include "asmfunc.h"
#include "bootpack.h"

volatile unsigned long jiffies = 0;

void init_pit(void) {
    // 芯片主频为PIT_TICK_RATE，想要1ms触发1次，HZ为1000，设置PIT_LATCH = PIT_TICK_RATE / HZ
    io_out8(PIT_MODE, 0x34);
    io_out8(PIT_CH0, PIT_LATCH & 0xff);
    io_out8(PIT_CH0, PIT_LATCH >> 8);
}

void inthandler20(int *esp) {
    io_out8(PIC0_OCW2, 0x60);  // 通知PIC已经处理完毕
    ++jiffies;
}
