#include "asmfunc.h"

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
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)0x00270000;
    struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)0x0026f800;
    int i;

    // 初始化GDT
    for (i = 0; i < 8192; i++) {
        set_segmdesc(gdt + i, 0, 0, 0);
    }
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
    set_segmdesc(gdt + 2, 0x0000007f, 0x00280000, 0xc09a);
    load_gdtr(0xffff, 0x00270000);

    // 初始化IDT
    for (i = 0; i < 256; i++) {
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(0x7ff, 0x0026f800);

    return;
}
