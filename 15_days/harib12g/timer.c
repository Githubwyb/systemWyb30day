
#include <linux/i8253.h>
#include <linux/list.h>
#include <linux/timex.h>

#include "asmfunc.h"
#include "bootpack.h"

volatile unsigned long jiffies = 0;

static HLIST_HEAD(s_timer_list);  // 按照到期时间排序

void init_pit(void) {
    // 芯片主频为PIT_TICK_RATE，想要1ms触发1次，HZ为1000，设置PIT_LATCH = PIT_TICK_RATE / HZ
    io_out8(PIT_MODE, 0x34);
    io_out8(PIT_CH0, PIT_LATCH & 0xff);
    io_out8(PIT_CH0, PIT_LATCH >> 8);
}

void timer_free(struct TIMER *timer) {
    struct TIMER *node;
    timer->flags = 0;
    hlist_for_each_entry(node, &s_timer_list, entry) {
        if (node == timer) {
            hlist_del_init(&node->entry);
            break;
        }
    }
}

void timer_init(struct TIMER *timer, FIFO32Type *fifo, unsigned int data) {
    timer->fifo = fifo;
    timer->data = data;
    INIT_HLIST_NODE(&timer->entry);
}

void timer_settime(struct TIMER *timer, unsigned long timeout) {
    int e;
    timer->expires = timeout;

    // 操作s_timer_list需要关中断，防止并行操作
    e = io_load_eflags();
    io_cli();
    // 按照到期时间排序插入
    struct TIMER *node;
    struct TIMER *prev = NULL;  // 记录要插入的节点前一个的节点

    hlist_for_each_entry(node, &s_timer_list, entry) {
        if (node->expires > timer->expires) {
            break;
        }
        prev = node;
    }
    if (prev == NULL) {
        // 没有前一个节点，说明要插入到第一个
        // 1. 有节点，但是都不满足条件，插入最前面
        // 2. 没有节点，直接插入最前面
        hlist_add_head(&timer->entry, &s_timer_list);
    } else {
        // 有前一个节点，插入到前一个节点后面
        hlist_add_behind(&timer->entry, &prev->entry);
    }

    // 恢复中断
    io_store_eflags(e);
    io_sti();
}

void inthandler20(int *esp) {
    io_out8(PIC0_OCW2, 0x60);  // 通知PIC已经处理完毕
    ++jiffies;
    struct TIMER *node;
    struct hlist_node *n;
    bool ts = false;
    hlist_for_each_entry_safe(node, n, &s_timer_list, entry) {
        if (node->expires <= jiffies) {
            hlist_del_init(&node->entry);
            if (node == &g_mt_timer) {
                ts = true;
            } else {
                kfifo_put(node->fifo, node->data);
            }
        } else {
            break;
        }
    }

    if (ts) {
        mt_taskswitch();
    }
}
