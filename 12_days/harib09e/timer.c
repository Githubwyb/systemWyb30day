
#include <linux/i8253.h>
#include <linux/timex.h>

#include "asmfunc.h"
#include "bootpack.h"

volatile unsigned long jiffies = 0;

struct TIMERCTL timerctl;

#define TIMER_FLAGS_ALLOC 1  // 已经配置过
#define TIMER_FLAGS_USING 2  // 正在运行

void init_pit(void) {
    // 芯片主频为PIT_TICK_RATE，想要1ms触发1次，HZ为1000，设置PIT_LATCH = PIT_TICK_RATE / HZ
    io_out8(PIT_MODE, 0x34);
    io_out8(PIT_CH0, PIT_LATCH & 0xff);
    io_out8(PIT_CH0, PIT_LATCH >> 8);
    for (int i = 0; i < MAX_TIMER; ++i) {
        timerctl.timers[i].flags = 0;  // 未使用
    }
}

struct TIMER *timer_alloc(void) {
    for (int i = 0; i < MAX_TIMER; ++i) {
        if (timerctl.timers[i].flags == 0) {
            timerctl.timers[i].flags = TIMER_FLAGS_ALLOC;
            return &timerctl.timers[i];
        }
    }
    return NULL;
}

void timer_free(struct TIMER *timer) {
    timer->flags = 0;
}

void timer_init(struct TIMER *timer, TimerBufType *fifo, unsigned char data) {
    timer->fifo = fifo;
    timer->data = data;
}

void timer_settime(struct TIMER *timer, unsigned long timeout) {
    timer->timeout = timeout;
    timer->flags = TIMER_FLAGS_USING;
}

void inthandler20(int *esp) {
    io_out8(PIC0_OCW2, 0x60);  // 通知PIC已经处理完毕
    ++jiffies;
    for (int i = 0; i < MAX_TIMER; ++i) {
        if (timerctl.timers[i].flags == TIMER_FLAGS_USING) {
            if (timerctl.timers[i].timeout <= jiffies) {
                timerctl.timers[i].flags = TIMER_FLAGS_ALLOC;
                kfifo_put(timerctl.timers[i].fifo, timerctl.timers[i].data);
            }
        }
    }
}
