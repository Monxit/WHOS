; boot/isr.s
; Interrupt Service Routine stubs
; These save CPU state and call C handlers

BITS 32

; External C handlers
EXTERN isr_handler
EXTERN irq_handler

; ---------------------------
; GDT/IDT flush functions
; ---------------------------
GLOBAL gdt_flush
gdt_flush:
    mov eax, [esp+4]    ; Get pointer to GDT descriptor
    lgdt [eax]          ; Load GDT
    
    ; Reload segment registers
    mov ax, 0x10        ; Kernel data segment (index 2 * 8 = 0x10)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Far jump to reload CS with kernel code segment (0x08)
    jmp 0x08:.flush
.flush:
    ret

GLOBAL idt_flush
idt_flush:
    mov eax, [esp+4]    ; Get pointer to IDT descriptor
    lidt [eax]          ; Load IDT
    ret

; ---------------------------
; Macro to create ISR stub without error code
; ---------------------------
%macro ISR_NOERRCODE 1
GLOBAL isr%1
isr%1:
    cli
    push dword 0        ; Push dummy error code
    push dword %1       ; Push interrupt number
    jmp isr_common_stub
%endmacro

; ---------------------------
; Macro to create ISR stub with error code (CPU pushes it)
; ---------------------------
%macro ISR_ERRCODE 1
GLOBAL isr%1
isr%1:
    cli
    push dword %1       ; Push interrupt number (error code already pushed)
    jmp isr_common_stub
%endmacro

; ---------------------------
; Macro to create IRQ stub
; ---------------------------
%macro IRQ 2
GLOBAL irq%1
irq%1:
    cli
    push dword 0        ; Dummy error code
    push dword %2       ; Push interrupt number (32 + IRQ number)
    jmp irq_common_stub
%endmacro

; ---------------------------
; CPU Exceptions (ISRs 0-31)
; ---------------------------
ISR_NOERRCODE 0   ; Division by Zero
ISR_NOERRCODE 1   ; Debug
ISR_NOERRCODE 2   ; Non-Maskable Interrupt
ISR_NOERRCODE 3   ; Breakpoint
ISR_NOERRCODE 4   ; Overflow
ISR_NOERRCODE 5   ; Bound Range Exceeded
ISR_NOERRCODE 6   ; Invalid Opcode
ISR_NOERRCODE 7   ; Device Not Available
ISR_ERRCODE   8   ; Double Fault
ISR_NOERRCODE 9   ; Coprocessor Segment Overrun
ISR_ERRCODE   10  ; Invalid TSS
ISR_ERRCODE   11  ; Segment Not Present
ISR_ERRCODE   12  ; Stack-Segment Fault
ISR_ERRCODE   13  ; General Protection Fault
ISR_ERRCODE   14  ; Page Fault
ISR_NOERRCODE 15  ; Reserved
ISR_NOERRCODE 16  ; x87 Floating-Point Exception
ISR_ERRCODE   17  ; Alignment Check
ISR_NOERRCODE 18  ; Machine Check
ISR_NOERRCODE 19  ; SIMD Floating-Point Exception
ISR_NOERRCODE 20  ; Virtualization Exception
ISR_NOERRCODE 21  ; Reserved
ISR_NOERRCODE 22  ; Reserved
ISR_NOERRCODE 23  ; Reserved
ISR_NOERRCODE 24  ; Reserved
ISR_NOERRCODE 25  ; Reserved
ISR_NOERRCODE 26  ; Reserved
ISR_NOERRCODE 27  ; Reserved
ISR_NOERRCODE 28  ; Reserved
ISR_NOERRCODE 29  ; Reserved
ISR_ERRCODE   30  ; Security Exception
ISR_NOERRCODE 31  ; Reserved

; ---------------------------
; Hardware IRQs (IRQ 0-15 -> INT 32-47)
; ---------------------------
IRQ 0,  32  ; Programmable Interval Timer
IRQ 1,  33  ; Keyboard
IRQ 2,  34  ; Cascade (for slave PIC)
IRQ 3,  35  ; COM2
IRQ 4,  36  ; COM1
IRQ 5,  37  ; LPT2
IRQ 6,  38  ; Floppy Disk
IRQ 7,  39  ; LPT1 / Spurious
IRQ 8,  40  ; CMOS RTC
IRQ 9,  41  ; Free / ACPI
IRQ 10, 42  ; Free
IRQ 11, 43  ; Free
IRQ 12, 44  ; PS/2 Mouse
IRQ 13, 45  ; FPU / Coprocessor
IRQ 14, 46  ; Primary ATA
IRQ 15, 47  ; Secondary ATA

; ---------------------------
; Common ISR stub - saves state and calls C handler
; ---------------------------
isr_common_stub:
    pusha               ; Push edi, esi, ebp, esp, ebx, edx, ecx, eax

    mov ax, ds          ; Save data segment
    push eax

    mov ax, 0x10        ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp            ; Pass pointer to registers struct
    call isr_handler
    add esp, 4

    pop eax             ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                ; Restore registers
    add esp, 8          ; Clean up error code and interrupt number
    sti
    iret                ; Return from interrupt

; ---------------------------
; Common IRQ stub - saves state and calls C handler
; ---------------------------
irq_common_stub:
    pusha               ; Push all general-purpose registers

    mov ax, ds          ; Save data segment
    push eax

    mov ax, 0x10        ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp            ; Pass pointer to registers struct
    call irq_handler
    add esp, 4

    pop eax             ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                ; Restore registers
    add esp, 8          ; Clean up error code and interrupt number
    sti
    iret                ; Return from interrupt
