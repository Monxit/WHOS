; boot/boot.s
; Multiboot-compliant entry point for GRUB with graphics mode.
; NASM syntax, 32-bit code.

BITS 32
GLOBAL _start
EXTERN kernel_main

; ---------------------------
; Multiboot header constants
; ---------------------------
MULTIBOOT_MAGIC     equ 0x1BADB002

; Flags:
;   Bit 0 = align modules on page boundaries
;   Bit 1 = provide memory map
;   Bit 2 = provide video mode info (required for graphics)
MULTIBOOT_FLAGS     equ 0x00000007

; Checksum must make magic + flags + checksum == 0
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

; Desired video mode
VIDEO_MODE          equ 0          ; 0 = linear graphics mode
VIDEO_WIDTH         equ 800
VIDEO_HEIGHT        equ 600
VIDEO_DEPTH         equ 32

; ---------------------------
; Multiboot header (must be within first 8 KiB of the kernel image)
; ---------------------------
SECTION .multiboot
ALIGN 4
    dd MULTIBOOT_MAGIC          ; Magic number
    dd MULTIBOOT_FLAGS          ; Flags
    dd MULTIBOOT_CHECKSUM       ; Checksum
    
    ; Address fields (unused when flags bit 16 is not set)
    dd 0                        ; header_addr
    dd 0                        ; load_addr
    dd 0                        ; load_end_addr
    dd 0                        ; bss_end_addr
    dd 0                        ; entry_addr
    
    ; Video mode fields (used when flags bit 2 is set)
    dd VIDEO_MODE               ; mode_type (0=graphics, 1=text)
    dd VIDEO_WIDTH              ; width
    dd VIDEO_HEIGHT             ; height
    dd VIDEO_DEPTH              ; depth (bits per pixel)

; ---------------------------
; Code section
; ---------------------------
SECTION .text
_start:
    ; Disable interrupts initially
    cli
    
    ; Set up a simple stack
    mov esp, stack_top

    ; ---------------------------
    ; Enable FPU / SSE
    ; ---------------------------
    
    ; 1. Enable FPU (x87)
    mov eax, cr0
    and eax, 0xFFFFFFFB     ; Clear EM (Bit 2) - Emulation
    or  eax, 0x00000002     ; Set MP (Bit 1) - Monitor Coprocessor
    mov cr0, eax
    
    fninit                  ; Initialize x87 FPU state

    ; 2. Enable SSE
    mov eax, cr4
    or  eax, 0x00000600     ; Set OSFXSR (Bit 9) and OSXMMEXCPT (Bit 10)
    mov cr4, eax

    ; ---------------------------
    ; Pass multiboot info to kernel
    ; ---------------------------
    ; EBX contains pointer to multiboot info structure (from GRUB)
    ; Push it as argument to kernel_main
    push ebx
    
    ; Call the C kernel entry point
    call kernel_main
    
    ; Clean up stack
    add esp, 4

    ; If kernel_main returns, halt the CPU
.halt:
    cli
    hlt
    jmp .halt

; ---------------------------
; Stack (16 KiB - larger for GUI)
; ---------------------------
SECTION .bss
ALIGN 16
stack_bottom:
    resb 16384
stack_top:
