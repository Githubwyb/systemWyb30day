#ifndef _ASM_X86_DESC_H
#define _ASM_X86_DESC_H

#include <asm/desc_defs.h>

// TODO
#include <string.h>

#define write_gdt_entry(dt, entry, desc, type) native_write_gdt_entry(dt, entry, desc, type)

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

#endif  // _ASM_X86_DESC_H
