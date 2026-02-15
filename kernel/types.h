// kernel/types.h
// Common type definitions for the OS

#ifndef TYPES_H
#define TYPES_H

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef signed char        i8;
typedef signed short       i16;
typedef signed int         i32;
typedef signed long long   i64;

typedef u32 size_t;
typedef i32 ssize_t;

#define NULL ((void*)0)
#define true  1
#define false 0
typedef u8 bool;

#endif // TYPES_H
