#ifndef _LINUX_COMPILER_H
#define _LINUX_COMPILER_H

#include <linux/compiler_types.h>

/* &a[0] degrades to a pointer: a different type from an array */
#define __must_be_array(a) BUILD_BUG_ON_ZERO(__same_type((a), &(a)[0]))

/* Not-quite-unique ID. */
#ifndef __UNIQUE_ID
#define __UNIQUE_ID(prefix) __PASTE(__PASTE(__UNIQUE_ID_, prefix), __LINE__)
#endif

#endif  // _LINUX_COMPILER_H
