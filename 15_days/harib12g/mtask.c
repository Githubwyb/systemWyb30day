#include <linux/jiffies.h>

#include "bootpack.h"
#include "log.h"

struct TIMER g_mt_timer;
static int s_mt_tr;

static inline void farjmp(u16 seg, u16 offset) {
    __asm__ __volatile__("ljmp *%0\n"
                         :
                         : "m"((struct {
                             unsigned int off;
                             unsigned short sel;
                         }){offset, seg}));
}

void mt_timer_init(void) {
    timer_init(&g_mt_timer, NULL, 0);
    timer_settime(&g_mt_timer, jiffies + msecs_to_jiffies(20));
    s_mt_tr = 1 * 8;
}

void mt_taskswitch(void) {
    if (s_mt_tr == 1 * 8) {
        s_mt_tr = 2 * 8;
    } else {
        s_mt_tr = 1 * 8;
    }
    timer_settime(&g_mt_timer, jiffies + msecs_to_jiffies(20));
    farjmp(s_mt_tr, 0);
}
