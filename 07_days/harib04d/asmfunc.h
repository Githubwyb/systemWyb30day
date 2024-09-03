#ifndef __ASM_FUNC_H__
#define __ASM_FUNC_H__

#include <linux/types.h>
#include <asm/io.h>
#include <asm/desc_defs.h>

/********** asmfunc.S中定义的函数 **********/
void io_hlt(void);
// 中断处理函数
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);

static inline void io_cli(void) {
    asm volatile("cli");
}

static inline void io_sti(void) {
    asm volatile("sti");
}

static inline void io_stihlt(void) {
    asm volatile("sti; hlt");
}

#define io_out8(port, val) outb(val, port)
#define io_out16(port, val) outw(val, port)
#define io_out32(port, val) outl(val, port)
#define io_in8(port) inb(port)
#define io_in16(port) inw(port)
#define io_in32(port) inl(port)

static inline uint32_t io_load_eflags(void) {
    uint32_t eflags;
    asm volatile("pushfl; popl %0" : "=r" (eflags));
    return eflags;
}

static inline void io_store_eflags(uint32_t eflags) {
    asm volatile("pushl %0; popfl" : : "r" (eflags));
}

static inline void native_load_gdt(const struct desc_ptr *dtr)
{
	asm volatile("lgdt %0"::"m" (*dtr));
}

static __always_inline void native_load_idt(const struct desc_ptr *dtr)
{
	asm volatile("lidt %0"::"m" (*dtr));
}

static inline void load_gdtr(u16 size, u32 address) {
    struct desc_ptr dtr_desc = {
        .size = size,
        .address  = address,
    };
    native_load_gdt(&dtr_desc);
}

static inline void load_idtr(u16 size, u32 address) {
    struct desc_ptr dtr_desc = {
        .size = size,
        .address  = address,
    };
    native_load_idt(&dtr_desc);
}

#endif  // __ASM_FUNC_H__
