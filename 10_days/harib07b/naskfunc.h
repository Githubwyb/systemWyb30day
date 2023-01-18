/**
 * @file naskfunc.h
 * @author wangyubo (1061322005@qq.com)
 * @brief 声明外部汇编函数
 * @version 0.1
 * @date 2022-10-15
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef __NASKFUNC_H__
#define __NASKFUNC_H__

/**
 * @brief cpu休眠，等待外部输入唤醒
 *
 */
void io_hlt();
/**
 * @brief 停止中断
 *
 */
void io_cli();
/**
 * @brief 开启中断
 *
 */
void io_sti();
void io_stihlt();
int io_in8(int port);
void io_out8(int port, int data);
int io_load_eflags();
void io_store_eflags(int eflags);

// 中断处理函数
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
/**
 * @brief 加载gdt
 *
 * @param limit gdt字节数
 * @param addr gdt在内存的首地址
 */
void load_gdtr(int limit, int addr);
/**
 * @brief 加载idt
 *
 * @param limit idt字节数
 * @param addr idt在内存的首地址
 */
void load_idtr(int limit, int addr);
int load_cr0(void);
void store_cr0(int cr0);
unsigned int memtest_sub(unsigned int start, unsigned int end);

#endif  // __NASKFUNC_H__
