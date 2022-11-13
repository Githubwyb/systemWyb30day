#include <asm/desc.h>
#include <asm/desc_defs.h>
#include <linux/kernel.h>

#include "naskfunc.h"

#define ADR_IDT 0x0026f800       // IDT的内存位置
#define LIMIT_IDT 0x000007ff     // IDT占用的字节数
#define ADR_GDT 0x00270000       // GDT的内存位置
#define LIMIT_GDT 0x0000ffff     // GDT占用的字节数
#define ADR_BOTPAK 0x00280000    // bootpack.hrb所在的地址
#define LIMIT_BOTPAK 0x0007f     // bootpack.hrb最大为 4k x 128 = 512k

// 全局gdt表
static const struct gdt_page gdt_page = {.gdt = {
#ifdef CONFIG_X86_64
#else
                                             // kernel的代码段，也就是bootpack所在位置
                                             [GDT_ENTRY_KERNEL_CS] = GDT_ENTRY_INIT(0xc09a, ADR_BOTPAK, LIMIT_BOTPAK),
                                             // kernel控制的总内存大小
                                             [GDT_ENTRY_KERNEL_DS] = GDT_ENTRY_INIT(0xc092, 0, 0xfffff),
#endif
                                         }};

#define DPL0 0x0
#define DPL3 0x3

#define DEFAULT_STACK 0

#define G(_vector, _addr, _ist, _type, _dpl, _segment)                                                         \
    {                                                                                                          \
        .vector = _vector, .bits.ist = _ist, .bits.type = _type, .bits.dpl = _dpl, .bits.p = 1, .addr = _addr, \
        .segment = _segment,                                                                                   \
    }

/* Interrupt gate */
#define INTG(_vector, _addr) G(_vector, _addr, DEFAULT_STACK, GATE_INTERRUPT, DPL0, __KERNEL_CS)

/* System interrupt gate */
#define SYSG(_vector, _addr) G(_vector, _addr, DEFAULT_STACK, GATE_INTERRUPT, DPL3, __KERNEL_CS)

// idt表
static const struct idt_data idt_table[] = {
    // PS/2键盘中断
    INTG(0x21, asm_inthandler21),
    // 处理27中断
    INTG(0x27, asm_inthandler27),
    // PS/2鼠标中断
    INTG(0x2c, asm_inthandler2c),
};

void init_gdtidt(void) {
    int i;

    // 初始化GDT
    struct desc_struct *gdt = (struct desc_struct *)ADR_GDT;
    // bootpack.hrb的段
    write_gdt_entry(gdt, GDT_ENTRY_KERNEL_CS, &gdt_page.gdt[GDT_ENTRY_KERNEL_CS], DESCTYPE_S);
    // cpu管理的总内存
    write_gdt_entry(gdt, GDT_ENTRY_KERNEL_DS, &gdt_page.gdt[GDT_ENTRY_KERNEL_DS], DESCTYPE_S);
    // 设置gdtr
    load_gdtr(LIMIT_GDT, ADR_GDT);

    // 初始化IDT
    gate_desc *idt = (gate_desc *)ADR_IDT;
    gate_desc desc;
    struct idt_data empty = INTG(0, 0);
    // 所有中断位置设置为空
    for (i = 0; i < LIMIT_IDT / sizeof(gate_desc); i++) {
        idt_init_desc(&desc, &empty);
        write_idt_entry(idt, i, &desc);
    }
    // 初始化要使用的中断
    const struct idt_data *t = idt_table;
    for (i = 0; i < ARRAY_SIZE(idt_table); ++i, ++t) {
        idt_init_desc(&desc, t);
        write_idt_entry(idt, t->vector, &desc);
    }

    load_idtr(LIMIT_IDT, ADR_IDT);
    return;
}
