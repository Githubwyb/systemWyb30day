#ifndef _ASM_X86_DESC_H
#define _ASM_X86_DESC_H

#include <asm/desc_defs.h>

// TODO
#include <asm/segment.h>
#include <string.h>

struct gdt_page {
    struct desc_struct gdt[GDT_ENTRIES];
};
// TODO } __attribute__((aligned(PAGE_SIZE)));

#define write_gdt_entry(dt, entry, desc, type) native_write_gdt_entry(dt, entry, desc, type)
#define write_idt_entry(dt, entry, g) native_write_idt_entry(dt, entry, g)

static inline void native_write_idt_entry(gate_desc *idt, int entry, const gate_desc *gate) {
    memcpy(&idt[entry], gate, sizeof(*gate));
}

static inline void native_write_gdt_entry(struct desc_struct *gdt, int entry, const void *desc, int type) {
    unsigned int size;

    switch (type) {
        case DESC_TSS:
            size = sizeof(tss_desc);
            break;
        case DESC_LDT:
            size = sizeof(ldt_desc);
            break;
        default:
            size = sizeof(*gdt);
            break;
    }

    memcpy(&gdt[entry], desc, size);
}

static inline void init_idt_data(struct idt_data *data, unsigned int n, const void *addr) {
    // TODO BUG_ON(n > 0xFF);

    memset(data, 0, sizeof(*data));
    data->vector = n;
    data->addr = addr;
    data->segment = __KERNEL_CS;
    data->bits.type = GATE_INTERRUPT;
    data->bits.p = 1;
}

static inline void idt_init_desc(gate_desc *gate, const struct idt_data *d) {
    unsigned long addr = (unsigned long)d->addr;

    gate->offset_low = (u16)addr;
    gate->segment = (u16)d->segment;
    gate->bits = d->bits;
    gate->offset_middle = (u16)(addr >> 16);
#ifdef CONFIG_X86_64
    gate->offset_high = (u32)(addr >> 32);
    gate->reserved = 0;
#endif
}

#endif  // _ASM_X86_DESC_H
