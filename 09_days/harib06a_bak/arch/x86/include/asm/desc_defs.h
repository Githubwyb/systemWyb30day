#ifndef _ASM_X86_DESC_DEFS_H
#define _ASM_X86_DESC_DEFS_H

#include <linux/types.h>

/* 8 byte segment descriptor */
struct desc_struct {
    u16 limit0;
    u16 base0;
    u16 base1 : 8, type : 4, s : 1, dpl : 2, p : 1;
    u16 limit1 : 4, avl : 1, l : 1, d : 1, g : 1, base2 : 8;
} __attribute__((packed));

#define GDT_ENTRY_INIT(flags, base, limit)                                                                           \
    {                                                                                                                \
        .limit0 = (u16)(limit), .limit1 = ((limit) >> 16) & 0x0F, .base0 = (u16)(base),                              \
        .base1 = ((base) >> 16) & 0xFF, .base2 = ((base) >> 24) & 0xFF, .type = (flags & 0x0f),                      \
        .s = (flags >> 4) & 0x01, .dpl = (flags >> 5) & 0x03, .p = (flags >> 7) & 0x01, .avl = (flags >> 12) & 0x01, \
        .l = (flags >> 13) & 0x01, .d = (flags >> 14) & 0x01, .g = (flags >> 15) & 0x01,                             \
    }

enum {
    GATE_INTERRUPT = 0xE,
    GATE_TRAP = 0xF,
    GATE_CALL = 0xC,
    GATE_TASK = 0x5,
};

enum {
    DESC_TSS = 0x9,
    DESC_LDT = 0x2,
    DESCTYPE_S = 0x10, /* !system */
};

/* LDT or TSS descriptor in the GDT. */
struct ldttss_desc {
    u16 limit0;
    u16 base0;

    u16 base1 : 8, type : 5, dpl : 2, p : 1;
    u16 limit1 : 4, zero0 : 3, g : 1, base2 : 8;
#ifdef CONFIG_X86_64
    u32 base3;
    u32 zero1;
#endif
} __attribute__((packed));

typedef struct ldttss_desc ldt_desc;
typedef struct ldttss_desc tss_desc;

struct idt_bits {
    u16 ist : 3, zero : 5, type : 5, dpl : 2, p : 1;
} __attribute__((packed));

struct idt_data {
    unsigned int vector;
    unsigned int segment;
    struct idt_bits bits;
    const void *addr;
};

struct gate_struct {
    u16 offset_low;
    u16 segment;
    struct idt_bits bits;
    u16 offset_middle;
#ifdef CONFIG_X86_64
    u32 offset_high;
    u32 reserved;
#endif
} __attribute__((packed));

typedef struct gate_struct gate_desc;

#endif  // _ASM_X86_DESC_DEFS_H
