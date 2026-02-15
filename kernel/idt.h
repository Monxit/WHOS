// kernel/idt.h
// Interrupt Descriptor Table - handles hardware and software interrupts

#ifndef IDT_H
#define IDT_H

#include "types.h"

// IDT entry structure (8 bytes each)
struct idt_entry {
    u16 base_low;       // Lower 16 bits of handler address
    u16 selector;       // Kernel code segment selector
    u8  zero;           // Must be zero
    u8  flags;          // Type and attributes
    u16 base_high;      // Upper 16 bits of handler address
} __attribute__((packed));

// IDT pointer structure for LIDT instruction
struct idt_ptr {
    u16 limit;          // Size of IDT - 1
    u32 base;           // Address of the IDT
} __attribute__((packed));

// Registers pushed by ISR stubs
struct registers {
    u32 ds;                                     // Data segment
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha
    u32 int_no, err_code;                       // Interrupt number and error code
    u32 eip, cs, eflags, useresp, ss;           // Pushed by CPU
};

// Function pointer type for interrupt handlers
typedef void (*isr_handler_t)(struct registers*);

void idt_init(void);
void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags);
void register_interrupt_handler(u8 n, isr_handler_t handler);

#endif // IDT_H
