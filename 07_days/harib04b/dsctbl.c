#include "asmfunc.h"

#define ADR_IDT 0x0026f800       // IDT的内存位置
#define LIMIT_IDT 0x000007ff     // IDT占用的字节数
#define ADR_GDT 0x00270000       // GDT的内存位置
#define LIMIT_GDT 0x0000ffff     // GDT占用的字节数
#define ADR_BOTPAK 0x00280000    // bootpack.hrb所在的地址
#define LIMIT_BOTPAK 0x0007ffff  // bootpack.hrb最大为512k
#define AR_DATA32_RW 0x4092      // 数据段，可读写
#define AR_CODE32_ER 0x409a      // 代码段，可读可执行，不可写
#define AR_INTGATE32 0x008e      // 中断处理的属性

// 根据cpu手册定义，可以参考 https://blog.csdn.net/m0_46125480/article/details/120381165
struct SEGMENT_DESCRIPTOR {
    short limit_low;  // 段上限low
    short base_low;   // 段地址low
    char base_mid;    // 段地址mid
    char access_right;
    char limit_high;  // 段上限high
    char base_high;   // 段地址high
};

struct GATE_DESCRIPTOR {
    short offset_low;
    short selector;
    char dw_count;
    char access_right;
    short offset_high;
};

static void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar) {
    if (limit > 0xfffff) {
        ar |= 0x8000; /* G_bit = 1 */
        limit /= 0x1000;
    }
    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high = (base >> 24) & 0xff;
    return;
}

static void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar) {
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
    return;
}

void init_gdtidt(void) {
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
    struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)ADR_IDT;
    int i;

    // 初始化GDT
    for (i = 0; i < LIMIT_GDT / sizeof(struct SEGMENT_DESCRIPTOR); i++) {
        set_segmdesc(gdt + i, 0, 0, 0);
    }
    // cpu管理的总内存
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);
    // 汇编的段
    set_segmdesc(gdt + 2, 0x000fffff, 0x00000000, AR_CODE32_ER);
    // c语言的段
    set_segmdesc(gdt + 3, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);
    load_gdtr(LIMIT_GDT, ADR_GDT);

    // 初始化IDT
    for (i = 0; i < LIMIT_IDT / sizeof(struct GATE_DESCRIPTOR); i++) {
        set_gatedesc(idt + i, 0, 0, 0);
    }

    // 注册中断处理函数
    set_gatedesc(idt + 0x21, (int)asm_inthandler21-ADR_BOTPAK, 3 * 8, AR_INTGATE32);
    set_gatedesc(idt + 0x27, (int)asm_inthandler27-ADR_BOTPAK, 3 * 8, AR_INTGATE32);
    set_gatedesc(idt + 0x2c, (int)asm_inthandler2c-ADR_BOTPAK, 3 * 8, AR_INTGATE32);

    load_idtr(LIMIT_IDT, ADR_IDT);
    return;
}
