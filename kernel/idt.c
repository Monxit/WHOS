// kernel/idt.c
// Interrupt Descriptor Table implementation

#include "idt.h"
#include "io.h"

#define IDT_ENTRIES 256

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr   idtp;

// Handler function pointers
static isr_handler_t interrupt_handlers[IDT_ENTRIES];

// External assembly function to load the IDT
extern void idt_flush(u32 idt_ptr);

// ISR stubs (defined in isr.s)
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

// IRQ stubs (defined in isr.s)
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags) {
    idt[num].base_low  = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector  = sel;
    idt[num].zero      = 0;
    idt[num].flags     = flags;
}

// Remap the PIC (Programmable Interrupt Controller)
// By default, IRQs 0-7 map to interrupts 0-7 (conflict with CPU exceptions)
// We remap them to 32-47
static void pic_remap(void) {
    // Save masks
    u8 a1 = inb(0x21);
    u8 a2 = inb(0xA1);

    // Start initialization sequence (ICW1)
    outb(0x20, 0x11); io_wait();
    outb(0xA0, 0x11); io_wait();

    // ICW2: Set vector offsets
    outb(0x21, 0x20); io_wait();  // Master PIC: IRQ 0-7 -> INT 32-39
    outb(0xA1, 0x28); io_wait();  // Slave PIC:  IRQ 8-15 -> INT 40-47

    // ICW3: Tell Master about Slave on IRQ2
    outb(0x21, 0x04); io_wait();
    outb(0xA1, 0x02); io_wait();

    // ICW4: 8086 mode
    outb(0x21, 0x01); io_wait();
    outb(0xA1, 0x01); io_wait();

    // Restore masks
    outb(0x21, a1);
    outb(0xA1, a2);
}

void idt_init(void) {
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base  = (u32)&idt;

    // Clear all entries
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
        interrupt_handlers[i] = 0;
    }

    // Remap the PIC
    pic_remap();

    // Set up CPU exception handlers (ISRs 0-31)
    // 0x08 = kernel code segment, 0x8E = present, ring 0, 32-bit interrupt gate
    idt_set_gate(0,  (u32)isr0,  0x08, 0x8E);
    idt_set_gate(1,  (u32)isr1,  0x08, 0x8E);
    idt_set_gate(2,  (u32)isr2,  0x08, 0x8E);
    idt_set_gate(3,  (u32)isr3,  0x08, 0x8E);
    idt_set_gate(4,  (u32)isr4,  0x08, 0x8E);
    idt_set_gate(5,  (u32)isr5,  0x08, 0x8E);
    idt_set_gate(6,  (u32)isr6,  0x08, 0x8E);
    idt_set_gate(7,  (u32)isr7,  0x08, 0x8E);
    idt_set_gate(8,  (u32)isr8,  0x08, 0x8E);
    idt_set_gate(9,  (u32)isr9,  0x08, 0x8E);
    idt_set_gate(10, (u32)isr10, 0x08, 0x8E);
    idt_set_gate(11, (u32)isr11, 0x08, 0x8E);
    idt_set_gate(12, (u32)isr12, 0x08, 0x8E);
    idt_set_gate(13, (u32)isr13, 0x08, 0x8E);
    idt_set_gate(14, (u32)isr14, 0x08, 0x8E);
    idt_set_gate(15, (u32)isr15, 0x08, 0x8E);
    idt_set_gate(16, (u32)isr16, 0x08, 0x8E);
    idt_set_gate(17, (u32)isr17, 0x08, 0x8E);
    idt_set_gate(18, (u32)isr18, 0x08, 0x8E);
    idt_set_gate(19, (u32)isr19, 0x08, 0x8E);
    idt_set_gate(20, (u32)isr20, 0x08, 0x8E);
    idt_set_gate(21, (u32)isr21, 0x08, 0x8E);
    idt_set_gate(22, (u32)isr22, 0x08, 0x8E);
    idt_set_gate(23, (u32)isr23, 0x08, 0x8E);
    idt_set_gate(24, (u32)isr24, 0x08, 0x8E);
    idt_set_gate(25, (u32)isr25, 0x08, 0x8E);
    idt_set_gate(26, (u32)isr26, 0x08, 0x8E);
    idt_set_gate(27, (u32)isr27, 0x08, 0x8E);
    idt_set_gate(28, (u32)isr28, 0x08, 0x8E);
    idt_set_gate(29, (u32)isr29, 0x08, 0x8E);
    idt_set_gate(30, (u32)isr30, 0x08, 0x8E);
    idt_set_gate(31, (u32)isr31, 0x08, 0x8E);

    // Set up hardware IRQ handlers (IRQs 0-15 -> INTs 32-47)
    idt_set_gate(32, (u32)irq0,  0x08, 0x8E);  // Timer
    idt_set_gate(33, (u32)irq1,  0x08, 0x8E);  // Keyboard
    idt_set_gate(34, (u32)irq2,  0x08, 0x8E);
    idt_set_gate(35, (u32)irq3,  0x08, 0x8E);
    idt_set_gate(36, (u32)irq4,  0x08, 0x8E);
    idt_set_gate(37, (u32)irq5,  0x08, 0x8E);
    idt_set_gate(38, (u32)irq6,  0x08, 0x8E);
    idt_set_gate(39, (u32)irq7,  0x08, 0x8E);
    idt_set_gate(40, (u32)irq8,  0x08, 0x8E);
    idt_set_gate(41, (u32)irq9,  0x08, 0x8E);
    idt_set_gate(42, (u32)irq10, 0x08, 0x8E);
    idt_set_gate(43, (u32)irq11, 0x08, 0x8E);
    idt_set_gate(44, (u32)irq12, 0x08, 0x8E);  // Mouse
    idt_set_gate(45, (u32)irq13, 0x08, 0x8E);
    idt_set_gate(46, (u32)irq14, 0x08, 0x8E);
    idt_set_gate(47, (u32)irq15, 0x08, 0x8E);

    // Load the IDT
    idt_flush((u32)&idtp);
}

void register_interrupt_handler(u8 n, isr_handler_t handler) {
    interrupt_handlers[n] = handler;
}

// Called from assembly - common handler for all interrupts
void isr_handler(struct registers* regs) {
    if (interrupt_handlers[regs->int_no]) {
        interrupt_handlers[regs->int_no](regs);
    }
}

// Called from assembly - common handler for IRQs
void irq_handler(struct registers* regs) {
    // Send EOI (End of Interrupt) to PICs
    if (regs->int_no >= 40) {
        // Slave PIC
        outb(0xA0, 0x20);
    }
    // Master PIC
    outb(0x20, 0x20);

    // Call registered handler
    if (interrupt_handlers[regs->int_no]) {
        interrupt_handlers[regs->int_no](regs);
    }
}
