#include "bootpack.h"
#include "naskfunc.h"

#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000

unsigned int memtest(unsigned int start, unsigned int end) {
    char flg486 = 0;
    unsigned int eflg, cr0, i;

    /* 确认是386还是486以后 */
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    if ((eflg & EFLAGS_AC_BIT) != 0) { /* 在386中，即使AC=1，也会自动返回0。 */
        flg486 = 1;
    }
    eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
    io_store_eflags(eflg);

    if (flg486 != 0) {
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE; /* 禁止缓存 */
        store_cr0(cr0);
    }

    i = memtest_sub(start, end);

    if (flg486 != 0) {
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE; /* 高速缓存许可 */
        store_cr0(cr0);
    }

    return i;
}

void memman_init(struct MEMMAN *man) {
    man->frees = 0;
    man->maxfrees = 0;
    man->lostsize = 0;
    man->losts = 0;
}

/**
 * @brief 获取剩余内存大小
 *
 * @param man
 * @return uint32_t
 */
uint32_t memman_total(struct MEMMAN *man) {
    uint32_t i = 0;
    uint32_t t = 0;
    for (i = 0; i < man->frees; ++i) {
        t += man->free[i].size;
    }
    return t;
}

uint32_t memman_alloc(struct MEMMAN *man, uint32_t size) {
    uint32_t i = 0;
    uint32_t a = 0;
    for (i = 0; i < man->frees; ++i) {
        if (man->free[i].size >= size) {
            // 找到足够大的内存
            a = man->free[i].addr;
            man->free[i].addr += size;
            man->free[i].size -= size;
            if (man->free[i].size == 0) {
                // free变成0，就将free去掉
                for (; i < man->frees; ++i) {
                    man->free[i] = man->free[i + 1];
                }
            }
            return a;
        }
    }
    return 0;
}

int memman_free(struct MEMMAN *man, uint32_t addr, uint32_t size) {
    int32_t i;
    int32_t j;
    // 为了便于归纳内存，将free[]按照addr的顺序排列
    // 先决定放哪里
    for (i = 0; i < man->frees; ++i) {
        if (man->free[i].addr > addr) {
            break;
        }
    }
    // free[i-1].addr < addr < free[i].addr
    if (i > 0 && man->free[i - 1].addr + man->free[i - 1].size == addr) {
        // 可以与前面的内存归纳到一起
        man->free[i - 1].size += size;
        if (i < man->frees) {
            if (addr + size == man->free[i].addr) {
                // 后面下一个块也可以归纳到一起
                man->free[i - 1].size += man->free[i].size;
                man->frees--;
                for (; i < man->frees - 1; ++i) {
                    man->free[i] = man->free[i + 1];
                }
            }
        }
        return 0;
    }
    // 不能与前面归纳到一起但是可以和后面归纳到一起
    if (i < man->frees && addr + size == man->free[i].addr) {
        man->free[i].addr = addr;
        man->free[i].size += size;
        return 0;
    }
    // 既不能和前面归纳到一起，也不能和后面归纳到一起，新建一个info
    if (man->frees < MEMMAN_FREES) {
        // 腾个位置
        for (j = man->frees; i > i; --j) {
            man->free[j] = man->free[j - 1];
        }
        ++man->frees;
        if (man->maxfrees < man->frees) {
            man->maxfrees = man->frees;
        }
        man->free[i].addr = addr;
        man->free[i].size = size;
        return 0;
    }
    // 都不行，内存丢弃
    ++man->losts;
    man->lostsize += size;
    return -1;
}

u32 memman_alloc_4k(struct MEMMAN *man, u32 size) {
    size = (size + 0xfff) & ~0xfff;
    return memman_alloc(man, size);
}

u32 memman_free_4k(struct MEMMAN *man, u32 addr, u32 size) {
    size = (size + 0xfff) & ~0xfff;
    return memman_free(man, addr, size);
}
