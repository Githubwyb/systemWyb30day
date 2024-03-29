#ifndef _ASM_X86_SEGMENT_H
#define _ASM_X86_SEGMENT_H

#ifdef CONFIG_X86_32
/*                                                                    \
 * The layout of the per-CPU GDT under Linux:                         \
 *                                                                    \
 *   0 - null                               <=== cacheline #1         \
 *   1 - reserved                                                     \
 *   2 - reserved                                                     \
 *   3 - reserved                                                     \
 *                                                                    \
 *   4 - unused                             <=== cacheline #2         \
 *   5 - unused                                                       \
 *                                                                    \
 *  ------- start of TLS (Thread-Local Storage) segments:             \
 *                                                                    \
 *   6 - TLS segment #1         [ glibc's TLS segment ]               \
 *   7 - TLS segment #2         [ Wine's %fs Win32 segment ]          \
 *   8 - TLS segment #3                         <=== cacheline #3     \
 *   9 - reserved                                                     \
 *  10 - reserved                                                     \
 *  11 - reserved                                                     \
 *                                                                    \
 *  ------- start of kernel segments:                                 \
 *                                                                    \
 *  12 - kernel code segment                        <=== cacheline #4 \
 *  13 - kernel data segment                                          \
 *  14 - default user CS                                              \
 *  15 - default user DS                                              \
 *  16 - TSS                                <=== cacheline #5         \
 *  17 - LDT                                                          \
 *  18 - PNPBIOS support (16->32 gate)                                \
 *  19 - PNPBIOS support                                              \
 *  20 - PNPBIOS support                        <=== cacheline #6     \
 *  21 - PNPBIOS support                                              \
 *  22 - PNPBIOS support                                              \
 *  23 - APM BIOS support                                             \
 *  24 - APM BIOS support                       <=== cacheline #7     \
 *  25 - APM BIOS support                                             \
 *                                                                    \
 *  26 - ESPFIX small SS                                              \
 *  27 - per-cpu            [ offset to per-cpu data area ]           \
 *  28 - unused                                                       \
 *  29 - unused                                                       \
 *  30 - unused                                                       \
 *  31 - TSS for double fault handler                                 \
 */
#define GDT_ENTRY_TLS_MIN 6
#define GDT_ENTRY_TLS_MAX (GDT_ENTRY_TLS_MIN + GDT_ENTRY_TLS_ENTRIES - 1)

// #define GDT_ENTRY_KERNEL_CS 12
#define GDT_ENTRY_KERNEL_CS 2  // TODO 当前bootpack为第2个段
// #define GDT_ENTRY_KERNEL_DS 13
#define GDT_ENTRY_KERNEL_DS 1  // TODO cpu管理内存数量设置在第1个段
#define GDT_ENTRY_DEFAULT_USER_CS 14
#define GDT_ENTRY_DEFAULT_USER_DS 15
#define GDT_ENTRY_TSS 16
#define GDT_ENTRY_LDT 17
#define GDT_ENTRY_PNPBIOS_CS32 18
#define GDT_ENTRY_PNPBIOS_CS16 19
#define GDT_ENTRY_PNPBIOS_DS 20
#define GDT_ENTRY_PNPBIOS_TS1 21
#define GDT_ENTRY_PNPBIOS_TS2 22
#define GDT_ENTRY_APMBIOS_BASE 23

#define GDT_ENTRY_ESPFIX_SS 26
#define GDT_ENTRY_PERCPU 27

#define GDT_ENTRY_DOUBLEFAULT_TSS 31

/*
 * Number of entries in the GDT table:
 */
#define GDT_ENTRIES 32

/*
 * Segment selector values corresponding to the above entries:
 */

#define __KERNEL_CS (GDT_ENTRY_KERNEL_CS * 8)
#define __KERNEL_DS (GDT_ENTRY_KERNEL_DS * 8)
#define __USER_DS (GDT_ENTRY_DEFAULT_USER_DS * 8 + 3)
#define __USER_CS (GDT_ENTRY_DEFAULT_USER_CS * 8 + 3)
#define __ESPFIX_SS (GDT_ENTRY_ESPFIX_SS * 8)

/* segment for calling fn: */
#define PNP_CS32 (GDT_ENTRY_PNPBIOS_CS32 * 8)
/* code segment for BIOS: */
#define PNP_CS16 (GDT_ENTRY_PNPBIOS_CS16 * 8)

/* "Is this PNP code selector (PNP_CS32 or PNP_CS16)?" */
#define SEGMENT_IS_PNP_CODE(x) (((x)&0xf4) == PNP_CS32)

/* data segment for BIOS: */
#define PNP_DS (GDT_ENTRY_PNPBIOS_DS * 8)
/* transfer data segment: */
#define PNP_TS1 (GDT_ENTRY_PNPBIOS_TS1 * 8)
/* another data segment: */
#define PNP_TS2 (GDT_ENTRY_PNPBIOS_TS2 * 8)

#ifdef CONFIG_SMP
#define __KERNEL_PERCPU (GDT_ENTRY_PERCPU * 8)
#else
#define __KERNEL_PERCPU 0
#endif

#else /* 64-bit: */

#include <asm/cache.h>

#define GDT_ENTRY_KERNEL32_CS 1
#define GDT_ENTRY_KERNEL_CS 2
#define GDT_ENTRY_KERNEL_DS 3

/*
 * We cannot use the same code segment descriptor for user and kernel mode,
 * not even in long flat mode, because of different DPL.
 *
 * GDT layout to get 64-bit SYSCALL/SYSRET support right. SYSRET hardcodes
 * selectors:
 *
 *   if returning to 32-bit userspace: cs = STAR.SYSRET_CS,
 *   if returning to 64-bit userspace: cs = STAR.SYSRET_CS+16,
 *
 * ss = STAR.SYSRET_CS+8 (in either case)
 *
 * thus USER_DS should be between 32-bit and 64-bit code selectors:
 */
#define GDT_ENTRY_DEFAULT_USER32_CS 4
#define GDT_ENTRY_DEFAULT_USER_DS 5
#define GDT_ENTRY_DEFAULT_USER_CS 6

/* Needs two entries */
#define GDT_ENTRY_TSS 8
/* Needs two entries */
#define GDT_ENTRY_LDT 10

#define GDT_ENTRY_TLS_MIN 12
#define GDT_ENTRY_TLS_MAX 14

#define GDT_ENTRY_CPUNODE 15

/*
 * Number of entries in the GDT table:
 */
#define GDT_ENTRIES 16

/*
 * Segment selector values corresponding to the above entries:
 *
 * Note, selectors also need to have a correct RPL,
 * expressed with the +3 value for user-space selectors:
 */
#define __KERNEL32_CS (GDT_ENTRY_KERNEL32_CS * 8)
#define __KERNEL_CS (GDT_ENTRY_KERNEL_CS * 8)
#define __KERNEL_DS (GDT_ENTRY_KERNEL_DS * 8)
#define __USER32_CS (GDT_ENTRY_DEFAULT_USER32_CS * 8 + 3)
#define __USER_DS (GDT_ENTRY_DEFAULT_USER_DS * 8 + 3)
#define __USER32_DS __USER_DS
#define __USER_CS (GDT_ENTRY_DEFAULT_USER_CS * 8 + 3)
#define __CPUNODE_SEG (GDT_ENTRY_CPUNODE * 8 + 3)

#endif

#endif  // _ASM_X86_SEGMENT_H
