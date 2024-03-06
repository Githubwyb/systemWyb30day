/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_X86_IO_H
#define _ASM_X86_IO_H

/*
 * This file contains the definitions for the x86 IO instructions
 * inb/inw/inl/outb/outw/outl and the "string versions" of the same
 * (insb/insw/insl/outsb/outsw/outsl). You can also use "pausing"
 * versions of the single-IO instructions (inb_p/inw_p/..).
 *
 * This file is not meant to be obfuscating: it's just complicated
 * to (a) handle it all in a way that makes gcc able to optimize it
 * as well as possible and (b) trying to avoid writing the same thing
 * over and over again with slight variations and possibly making a
 * mistake somewhere.
 */

/*
 * Thanks to James van Artsdalen for a better timing-fix than
 * the two short jumps: using outb's to a nonexistent port seems
 * to guarantee better timings even on fast machines.
 *
 * On the other hand, I'd like to be sure of a non-existent port:
 * I feel a bit unsafe about using 0x80 (should be safe, though)
 *
 *		Linus
 */

/*
 *  Bit simplified and optimized by Jan Hubicka
 *  Support of BIGMEM added by Gerhard Wichert, Siemens AG, July 1999.
 *
 *  isa_memset_io, isa_memcpy_fromio, isa_memcpy_toio added,
 *  isa_read[wl] and isa_write[wl] fixed
 *  - Arnaldo Carvalho de Melo <acme@conectiva.com.br>
 */

#define ARCH_HAS_IOREMAP_WC
#define ARCH_HAS_IOREMAP_WT

#include <asm/shared/io.h>

#define build_mmio_read(name, size, type, reg, barrier)                                            \
    static inline type name(const volatile void __iomem *addr) {                                   \
        type ret;                                                                                  \
        asm volatile("mov" size " %1,%0" : reg(ret) : "m"(*(volatile type __force *)addr)barrier); \
        return ret;                                                                                \
    }

#define build_mmio_write(name, size, type, reg, barrier)                                            \
    static inline void name(type val, volatile void __iomem *addr) {                                \
        asm volatile("mov" size " %0,%1" : : reg(val), "m"(*(volatile type __force *)addr)barrier); \
    }

#define readb readb
#define readw readw
#define readl readl
#define readb_relaxed(a) __readb(a)
#define readw_relaxed(a) __readw(a)
#define readl_relaxed(a) __readl(a)
#define __raw_readb __readb
#define __raw_readw __readw
#define __raw_readl __readl

#define writeb writeb
#define writew writew
#define writel writel
#define writeb_relaxed(v, a) __writeb(v, a)
#define writew_relaxed(v, a) __writew(v, a)
#define writel_relaxed(v, a) __writel(v, a)
#define __raw_writeb __writeb
#define __raw_writew __writew
#define __raw_writel __writel

#endif /* _ASM_X86_IO_H */
