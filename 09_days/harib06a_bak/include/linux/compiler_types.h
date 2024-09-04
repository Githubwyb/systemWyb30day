#ifndef __LINUX_COMPILER_TYPES_H
#define __LINUX_COMPILER_TYPES_H

/* Indirect macros required for expanded argument pasting, eg. __LINE__. */
#define ___PASTE(a, b) a##b
#define __PASTE(a, b) ___PASTE(a, b)

/* Attributes */
#include <linux/compiler_attributes.h>

/* Are two types/vars the same type (ignoring qualifiers)? */
#define __same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))

#endif  // __LINUX_COMPILER_TYPES_H
