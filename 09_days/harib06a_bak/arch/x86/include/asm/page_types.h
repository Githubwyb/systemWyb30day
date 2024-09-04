#ifndef _ASM_X86_PAGE_DEFS_H
#define _ASM_X86_PAGE_DEFS_H

#include <linux/const.h>

/* PAGE_SHIFT determines the page size */
#define PAGE_SHIFT 12
#define PAGE_SIZE (_AC(1, UL) << PAGE_SHIFT)
#define PAGE_MASK (~(PAGE_SIZE - 1))

#endif  // _ASM_X86_PAGE_DEFS_H