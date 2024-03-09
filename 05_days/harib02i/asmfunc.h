#ifndef __ASM_FUNC_H__
#define __ASM_FUNC_H__

#include <linux/types.h>
#include <asm/io.h>
#include <asm/desc_defs.h>

/********** asmfunc.S中定义的函数 **********/
void io_hlt(void);

static inline void io_cli(void) {
    asm volatile("cli");
}

static inline uint8_t io_in8(int port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a" (ret) : "Nd" (port));
    return ret;
}

static inline uint16_t io_in16(int port) {
    uint16_t ret;
    asm volatile("inw %1, %0" : "=a" (ret) : "Nd" (port));
    return ret;
}

static inline uint32_t io_in32(int port) {
    uint32_t ret;
    asm volatile("inl %1, %0" : "=a" (ret) : "Nd" (port));
    return ret;
}

#define io_out8(port, val) outb(val, port)
#define io_out16(port, val) outw(val, port)
#define io_out32(port, val) outl(val, port)
// uint32_t io_load_eflags(void);
// void io_store_eflags(uint32_t eflags);

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
