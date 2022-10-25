#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

// TODO
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

/* bsd */
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;

/* sysv */
typedef unsigned char unchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#ifndef __BIT_TYPES_DEFINED__
#define __BIT_TYPES_DEFINED__

typedef u8 u_int8_t;
typedef s8 int8_t;
typedef u16 u_int16_t;
typedef s16 int16_t;
typedef u32 u_int32_t;
typedef s32 int32_t;

#endif /* !(__BIT_TYPES_DEFINED__) */

typedef u8 uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;

#if defined(__GNUC__)
typedef u64 uint64_t;
typedef u64 u_int64_t;
typedef s64 int64_t;
#endif

#endif  // _LINUX_TYPES_H
