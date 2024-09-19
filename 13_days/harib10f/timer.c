
#include <linux/i8253.h>
#include <linux/list.h>
#include <linux/timex.h>

#include "asmfunc.h"
#include "bootpack.h"

volatile unsigned long jiffies = 0;

#define TIMER_FLAGS_ALLOC 1
#define TIMER_FLAGS_USING 2

struct TIMERCTL timerctl;

void init_pit(void) {
    // 芯片主频为PIT_TICK_RATE，想要1ms触发1次，HZ为1000，设置PIT_LATCH = PIT_TICK_RATE / HZ
    io_out8(PIT_MODE, 0x34);
    io_out8(PIT_CH0, PIT_LATCH & 0xff);
    io_out8(PIT_CH0, PIT_LATCH >> 8);
    timerctl.next = 0xffffffff;
    for (int i = 0; i < MAX_TIMER; i++) {
        timerctl.timer[i].flags = 0;
    }
}

struct TIMER *timer_alloc(void) {
    int i;
    for (i = 0; i < MAX_TIMER; i++) {
        if (timerctl.timer[i].flags == 0) {
            timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
            return &timerctl.timer[i];
        }
    }
    return 0;
}

void timer_free(struct TIMER *timer) { timer->flags = 0; }

void timer_init(struct TIMER *timer, TimerBufType *fifo, unsigned char data) {
    timer->fifo = fifo;
    timer->data = data;
}

void timer_settime(struct TIMER *timer, unsigned long timeout) {
    timer->timeout = jiffies + timeout;
    timer->flags = TIMER_FLAGS_USING;
    if (timerctl.next > timer->timeout) {
        timerctl.next = timer->timeout;
    }
}

void inthandler20(int *esp) {
    io_out8(PIC0_OCW2, 0x60);  // 通知PIC已经处理完毕
    ++jiffies;
    if (timerctl.next > jiffies) {
        return;
    }
    timerctl.next = 0xffffffff;
    for (int i = 0; i < MAX_TIMER; i++) {
        if (timerctl.timer[i].flags == TIMER_FLAGS_USING) {
            if (timerctl.timer[i].timeout <= jiffies) {
                timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
                kfifo_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
            } else {
                if (timerctl.next > timerctl.timer[i].timeout) {
                    timerctl.next = timerctl.timer[i].timeout;
                }
            }
        }
    }
}
