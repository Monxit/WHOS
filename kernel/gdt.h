// kernel/gdt.h
// Global Descriptor Table - defines memory segments for protected mode

#ifndef GDT_H
#define GDT_H

#include "types.h"

// GDT entry structure (8 bytes each)
struct gdt_entry {
    u16 limit_low;      // Lower 16 bits of segment limit
    u16 base_low;       // Lower 16 bits of base address
    u8  base_middle;    // Next 8 bits of base address
    u8  access;         // Access flags (present, privilege, type)
    u8  granularity;    // Granularity and upper 4 bits of limit
    u8  base_high;      // Upper 8 bits of base address
} __attribute__((packed));

// GDT pointer structure for LGDT instruction
struct gdt_ptr {
    u16 limit;          // Size of GDT - 1
    u32 base;           // Address of the GDT
} __attribute__((packed));

void gdt_init(void);

#endif // GDT_H
