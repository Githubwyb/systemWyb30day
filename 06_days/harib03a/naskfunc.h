#ifndef __NASKFUNC_H__
#define __NASKFUNC_H__

// 声明外部汇编函数
extern void io_hlt();
extern void io_cli();
extern void io_out8(int port, int data);
extern int io_load_eflags();
extern void io_store_eflags(int eflags);
extern void load_gdtr(int limit, int addr);
extern void load_idtr(int limit, int addr);

#endif  // __NASKFUNC_H__