#include <asm/desc.h>
#include <asm/desc_defs.h>

#include "naskfunc.h"

#define ADR_IDT 0x0026f800       // IDT的内存位置
#define LIMIT_IDT 0x000007ff     // IDT占用的字节数
#define ADR_GDT 0x00270000       // GDT的内存位置
#define LIMIT_GDT 0x0000ffff     // GDT占用的字节数
#define ADR_BOTPAK 0x00280000    // bootpack.hrb所在的地址
#define LIMIT_BOTPAK 0x0007ffff  // bootpack.hrb最大为512k
#define AR_DATA32_RW 0x4092      // 数据段，可读写
#define AR_CODE32_ER 0x409a      // 代码段，可读可执行，不可写
#define AR_INTGATE32 0x008e      // 中断处理的属性

struct GATE_DESCRIPTOR {
    short offset_low;   // 处理函数在段中的地址low
    short selector;     // 函数对应的段的偏移（基于设置的GDT的基地址），比如2号段就是2*8，因为一个段结构体是8个字节
    char dw_count;
    char access_right;
    short offset_high;  // 处理函数在段中的地址high
};

static void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar) {
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
    return;
}

void init_gdtidt(void) {
    struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)ADR_IDT;
    int i;

    // 初始化GDT
    struct desc_struct d = GDT_ENTRY_INIT(0, 0, 0);
    struct desc_struct *gdt = (struct desc_struct *)ADR_GDT;
    for (i = 0; i < LIMIT_GDT / sizeof(struct desc_struct); i++) {
        write_gdt_entry(&d, 0, gdt + i, DESCTYPE_S);
    }
    // cpu管理的总内存
    {
        struct desc_struct d = GDT_ENTRY_INIT(AR_DATA32_RW, 0x00000000, 0xffffffff);
        write_gdt_entry(&d, 0, gdt + 1, DESCTYPE_S);
    }
    // bootpack.hrb的段
    {
        struct desc_struct d = GDT_ENTRY_INIT(AR_CODE32_ER, ADR_BOTPAK, LIMIT_BOTPAK);
        write_gdt_entry(&d, 0, gdt + 2, DESCTYPE_S);
    }

    // 初始化IDT
    for (i = 0; i < LIMIT_IDT / sizeof(struct GATE_DESCRIPTOR); i++) {
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(LIMIT_IDT, ADR_IDT);

    // 注册中断处理函数
    set_gatedesc(idt + 0x21, (int)asm_inthandler21, 2 * 8, AR_INTGATE32);
    set_gatedesc(idt + 0x27, (int)asm_inthandler27, 2 * 8, AR_INTGATE32);
    set_gatedesc(idt + 0x2c, (int)asm_inthandler2c, 2 * 8, AR_INTGATE32);

    return;
}
