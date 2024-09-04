#ifndef __LINUX_COMPILER_ATTRIBUTES_H
#define __LINUX_COMPILER_ATTRIBUTES_H

/*
 *   gcc:
 * https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-warn_005funused_005fresult-function-attribute
 * clang: https://clang.llvm.org/docs/AttributeReference.html#nodiscard-warn-unused-result
 */
// TODO
// #define __must_check __attribute__((__warn_unused_result__))
#define __must_check

#endif  // __LINUX_COMPILER_ATTRIBUTES_H
