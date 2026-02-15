// kernel/io.h
// Port I/O functions for communicating with hardware

#ifndef IO_H
#define IO_H

#include "types.h"

// Read a byte from a port
static inline u8 inb(u16 port) {
    u8 result;
    __asm__ __volatile__("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Write a byte to a port
static inline void outb(u16 port, u8 data) {
    __asm__ __volatile__("outb %0, %1" : : "a"(data), "Nd"(port));
}

// Read a word (16-bit) from a port
static inline u16 inw(u16 port) {
    u16 result;
    __asm__ __volatile__("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Write a word (16-bit) to a port
static inline void outw(u16 port, u16 data) {
    __asm__ __volatile__("outw %0, %1" : : "a"(data), "Nd"(port));
}

// Small delay for I/O operations
static inline void io_wait(void) {
    outb(0x80, 0);
}

#endif // IO_H
