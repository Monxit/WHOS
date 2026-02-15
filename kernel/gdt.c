// kernel/gdt.c
// Global Descriptor Table implementation

#include "gdt.h"

// We need 5 entries: null, kernel code, kernel data, user code, user data
#define GDT_ENTRIES 5

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr   gp;

// External assembly function to load the GDT
extern void gdt_flush(u32 gdt_ptr);

// Set a GDT entry
static void gdt_set_gate(int num, u32 base, u32 limit, u8 access, u8 gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);

    gdt[num].access      = access;
}

void gdt_init(void) {
    // Set up the GDT pointer
    gp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gp.base  = (u32)&gdt;

    // Null descriptor (required by CPU)
    gdt_set_gate(0, 0, 0, 0, 0);

    // Kernel Code Segment: base=0, limit=4GB, executable, ring 0
    // Access: Present(1) | DPL 0(00) | Type(1) | Exec(1) | Direction(0) | RW(1) | Accessed(0)
    // = 0b10011010 = 0x9A
    // Granularity: 4KB blocks(1) | 32-bit(1) | 0 | 0 | limit[19:16]
    // = 0b11001111 = 0xCF
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // Kernel Data Segment: base=0, limit=4GB, writable, ring 0
    // Access: 0b10010010 = 0x92
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // User Code Segment: ring 3
    // Access: Present(1) | DPL 3(11) | Type(1) | Exec(1) | Direction(0) | RW(1) | Accessed(0)
    // = 0b11111010 = 0xFA
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    // User Data Segment: ring 3
    // = 0b11110010 = 0xF2
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    // Flush old GDT and install new one
    gdt_flush((u32)&gp);
}
