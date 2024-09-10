#ifndef __BOOTPACK_H__
#define __BOOTPACK_H__

#include <linux/kfifo.h>
#include <linux/types.h>

// boot相关信息，内存地址 0x0ff0-0x0fff
#define ADR_BOOTINFO 0x00000ff0
struct BOOTINFO {
    char cyls;
    char leds;
    char vmode;  // 颜色数目的信息，颜色的位数
    char reserve;
    short scrnx;  // 分辨率的x
    short scrny;  // 分辨率的y
    u8 *vram;     // 图像缓冲区开始地址
};

/* dsctbl.c */
void init_gdtidt();

/* keybord.c */
#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064
// 按键结构体
#define KEYBUF_KFIFO_SIZE 32
typedef STRUCT_KFIFO(unsigned char, KEYBUF_KFIFO_SIZE) KeyBufType;
extern KeyBufType g_keybuf;
/**
 * @brief 处理PS/2键盘的中断
 *
 * @param esp
 */
void inthandler21(int *esp);
/**
 * @brief 等待键盘控制器可以发送数据
 *
 */
void wait_KBC_sendready();
/**
 * @brief 键盘控制器的初始化
 *
 */
void init_keyboard();

/* mouse.c */
struct MOUSE_DEC {
    unsigned char buf[3];  // 记录数据
    uint8_t phase;         // 记录状态
    int x, y, btn;
};
// 鼠标结构体
#define MOUSEBUF_KFIFO_SIZE 128
typedef STRUCT_KFIFO(unsigned char, MOUSEBUF_KFIFO_SIZE) MouseBufType;
extern MouseBufType g_mouseBuf;
/**
 * @brief 处理PS/2鼠标的中断
 *
 * @param esp
 */
void inthandler2c(int *esp);
/**
 * @brief 解码data，如果解码完成返回true
 *
 * @param mdec
 * @param data
 * @return bool true和false
 */
bool mouse_decode(struct MOUSE_DEC *mdec, uint8_t dat);
/**
 * @brief 使能鼠标
 *
 */
void enable_mouse();

/* int.c */
#define PIC0_ICW1 0x0020
#define PIC0_OCW2 0x0020
#define PIC0_IMR 0x0021
#define PIC0_ICW2 0x0021
#define PIC0_ICW3 0x0021
#define PIC0_ICW4 0x0021
#define PIC1_ICW1 0x00a0
#define PIC1_OCW2 0x00a0
#define PIC1_IMR 0x00a1
#define PIC1_ICW2 0x00a1
#define PIC1_ICW3 0x00a1
#define PIC1_ICW4 0x00a1

void init_pic();

/* memory.c */
#define MEMMAN_FREES 4096  // (4+4)*4096 = 32768 = 32KB
#define MEMMAN_ADDR 0x003c0000
// 可用信息
struct FREEINFO {
    uint32_t addr;
    uint32_t size;
};

// 内存管理
struct MEMMAN {
    int32_t frees;                       // free信息表的占用总数
    int32_t maxfrees;                    // 用于观察可用状况：frees的最大值
    int32_t lostsize;                    // 释放失败的内存的大小总和
    int32_t losts;                       // 释放失败的次数
    struct FREEINFO free[MEMMAN_FREES];  // 所有free信息表
};
unsigned int memtest(unsigned int start, unsigned int end);
/**
 * @brief 释放内存
 *
 * @param man
 * @param addr
 * @param size
 * @return int 成功返回0，失败返回-1
 */
int memman_free(struct MEMMAN *man, uint32_t addr, uint32_t size);
void memman_init(struct MEMMAN *man);
uint32_t memman_total(struct MEMMAN *man);
/**
 * @brief 申请内存，按照4K对齐向上取整
 *
 * @param man
 * @param size 输入的size会按照4K向上取整
 * @return uint32_t 首地址
 */
uint32_t memman_alloc_4k(struct MEMMAN *man, uint32_t size);
/**
 * @brief 按照4K对齐释放内存
 *
 * @param man
 * @param addr
 * @param size
 * @return int 成功返回0，失败返回-1
 */
int memman_free_4k(struct MEMMAN *man, uint32_t addr, uint32_t size);

#endif  // __BOOTPACK_H__
