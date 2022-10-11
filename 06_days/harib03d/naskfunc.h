#ifndef __NASKFUNC_H__
#define __NASKFUNC_H__

// 声明外部汇编函数
extern void io_hlt();
extern void io_cli();
extern void io_out8(int port, int data);
extern int io_load_eflags();
extern void io_store_eflags(int eflags);
/**
 * @brief 加载gdt
 *
 * @param limit gdt字节数
 * @param addr gdt在内存的首地址
 */
extern void load_gdtr(int limit, int addr);
/**
 * @brief 加载idt
 *
 * @param limit idt字节数
 * @param addr idt在内存的首地址
 */
extern void load_idtr(int limit, int addr);

#endif  // __NASKFUNC_H__
