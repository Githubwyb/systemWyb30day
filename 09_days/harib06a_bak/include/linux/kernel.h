#ifndef _LINUX_KERNEL_H
#define _LINUX_KERNEL_H

#include <linux/build_bug.h>
#include <linux/compiler.h>
#include <linux/minmax.h>

/**
 * ARRAY_SIZE - get the number of elements in array @arr
 * @arr: array to be sized
 */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))

#endif  // _LINUX_KERNEL_H
