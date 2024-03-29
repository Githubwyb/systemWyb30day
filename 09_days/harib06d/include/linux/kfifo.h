#ifndef _LINUX_KFIFO_H
#define _LINUX_KFIFO_H

#include <linux/kernel.h>
// TODO
#include <stddef.h>

// TODO
#define smp_wmb()

struct __kfifo {
    unsigned int in;
    unsigned int out;
    unsigned int mask;
    unsigned int esize;
    void *data;
};

#define __STRUCT_KFIFO_COMMON(datatype, recsize, ptrtype) \
    union {                                               \
        struct __kfifo kfifo;                             \
        datatype *type;                                   \
        const datatype *const_type;                       \
        char (*rectype)[recsize];                         \
        ptrtype *ptr;                                     \
        ptrtype const *ptr_const;                         \
    }

#define __STRUCT_KFIFO(type, size, recsize, ptrtype)               \
    {                                                              \
        __STRUCT_KFIFO_COMMON(type, recsize, ptrtype);             \
        type buf[((size < 2) || (size & (size - 1))) ? -1 : size]; \
    }

#define STRUCT_KFIFO(type, size) struct __STRUCT_KFIFO(type, size, 0, type)

#define __STRUCT_KFIFO_PTR(type, recsize, ptrtype)     \
    {                                                  \
        __STRUCT_KFIFO_COMMON(type, recsize, ptrtype); \
        type buf[0];                                   \
    }

#define STRUCT_KFIFO_PTR(type) struct __STRUCT_KFIFO_PTR(type, 0, type)

/*
 * helper macro to distinguish between real in place fifo where the fifo
 * array is a part of the structure and the fifo type where the array is
 * outside of the fifo structure.
 */
#define __is_kfifo_ptr(fifo) (sizeof(*fifo) == sizeof(STRUCT_KFIFO_PTR(typeof(*(fifo)->type))))

/**
 * DECLARE_KFIFO - macro to declare a fifo object
 * @fifo: name of the declared fifo
 * @type: type of the fifo elements
 * @size: the number of elements in the fifo, this must be a power of 2
 */
#define DECLARE_KFIFO(fifo, type, size) STRUCT_KFIFO(type, size) fifo

/**
 * DEFINE_KFIFO - macro to define and initialize a fifo
 * @fifo: name of the declared fifo datatype
 * @type: type of the fifo elements
 * @size: the number of elements in the fifo, this must be a power of 2
 *
 * Note: the macro can be used for global and local fifo data type variables.
 */
#define DEFINE_KFIFO(fifo, type, size)                                                               \
    DECLARE_KFIFO(fifo, type, size) = (typeof(fifo)) {                                               \
        {                                                                                            \
            {                                                                                        \
                .in = 0, .out = 0, .mask = __is_kfifo_ptr(&(fifo)) ? 0 : ARRAY_SIZE((fifo).buf) - 1, \
                .esize = sizeof(*(fifo).buf), .data = __is_kfifo_ptr(&(fifo)) ? NULL : (fifo).buf,   \
            }                                                                                        \
        }                                                                                            \
    }

/**
 * kfifo_put - put data into the fifo
 * @fifo: address of the fifo to be used
 * @val: the data to be added
 *
 * This macro copies the given value into the fifo.
 * It returns 0 if the fifo was full. Otherwise it returns the number
 * processed elements.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these macro.
 */
#define kfifo_put(fifo, val)                                                              \
    ({                                                                                    \
        typeof((fifo) + 1) __tmp = (fifo);                                                \
        typeof(*__tmp->const_type) __val = (val);                                         \
        unsigned int __ret;                                                               \
        size_t __recsize = sizeof(*__tmp->rectype);                                       \
        struct __kfifo *__kfifo = &__tmp->kfifo;                                          \
        if (__recsize)                                                                    \
            __ret = __kfifo_in_r(__kfifo, &__val, sizeof(__val), __recsize);              \
        else {                                                                            \
            __ret = !kfifo_is_full(__tmp);                                                \
            if (__ret) {                                                                  \
                (__is_kfifo_ptr(__tmp) ? ((typeof(__tmp->type))__kfifo->data)             \
                                       : (__tmp->buf))[__kfifo->in & __tmp->kfifo.mask] = \
                    *(typeof(__tmp->type))&__val;                                         \
                smp_wmb();                                                                \
                __kfifo->in++;                                                            \
            }                                                                             \
        }                                                                                 \
        __ret;                                                                            \
    })

/**
 * kfifo_get - get data from the fifo
 * @fifo: address of the fifo to be used
 * @val: address where to store the data
 *
 * This macro reads the data from the fifo.
 * It returns 0 if the fifo was empty. Otherwise it returns the number
 * processed elements.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these macro.
 */
#define kfifo_get(fifo, val)                                                                  \
    __kfifo_uint_must_check_helper(({                                                         \
        typeof((fifo) + 1) __tmp = (fifo);                                                    \
        typeof(__tmp->ptr) __val = (val);                                                     \
        unsigned int __ret;                                                                   \
        const size_t __recsize = sizeof(*__tmp->rectype);                                     \
        struct __kfifo *__kfifo = &__tmp->kfifo;                                              \
        if (__recsize)                                                                        \
            __ret = __kfifo_out_r(__kfifo, __val, sizeof(*__val), __recsize);                 \
        else {                                                                                \
            __ret = !kfifo_is_empty(__tmp);                                                   \
            if (__ret) {                                                                      \
                *(typeof(__tmp->type))__val =                                                 \
                    (__is_kfifo_ptr(__tmp) ? ((typeof(__tmp->type))__kfifo->data)             \
                                           : (__tmp->buf))[__kfifo->out & __tmp->kfifo.mask]; \
                smp_wmb();                                                                    \
                __kfifo->out++;                                                               \
            }                                                                                 \
        }                                                                                     \
        __ret;                                                                                \
    }))

static inline unsigned int __must_check __kfifo_uint_must_check_helper(unsigned int val) { return val; }

/**
 * kfifo_len - returns the number of used elements in the fifo
 * @fifo: address of the fifo to be used
 */
#define kfifo_len(fifo)                       \
    ({                                        \
        typeof((fifo) + 1) __tmpl = (fifo);   \
        __tmpl->kfifo.in - __tmpl->kfifo.out; \
    })

/**
 * kfifo_is_empty - returns true if the fifo is empty
 * @fifo: address of the fifo to be used
 */
#define kfifo_is_empty(fifo)                   \
    ({                                         \
        typeof((fifo) + 1) __tmpq = (fifo);    \
        __tmpq->kfifo.in == __tmpq->kfifo.out; \
    })

/**
 * kfifo_is_full - returns true if the fifo is full
 * @fifo: address of the fifo to be used
 */
#define kfifo_is_full(fifo)                     \
    ({                                          \
        typeof((fifo) + 1) __tmpq = (fifo);     \
        kfifo_len(__tmpq) > __tmpq->kfifo.mask; \
    })

extern unsigned int __kfifo_in_r(struct __kfifo *fifo, const void *buf, unsigned int len, size_t recsize);

extern unsigned int __kfifo_out_r(struct __kfifo *fifo, void *buf, unsigned int len, size_t recsize);

#endif  // _LINUX_KFIFO_H
