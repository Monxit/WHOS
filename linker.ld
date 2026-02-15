/* linker.ld
 * Linker script for a 32-bit freestanding kernel loaded at 1 MiB.
 */

ENTRY(_start)

SECTIONS
{
    /* Load the kernel at 1 MiB (0x00100000) */
    . = 0x00100000;

    /* Multiboot header must be early in the image */
    .multiboot ALIGN(4) : {
        *(.multiboot)
    }

    /* Code section */
    .text ALIGN(4) : {
        *(.text*)
    }

    /* Read-only data */
    .rodata ALIGN(4) : {
        *(.rodata*)
    }

    /* Initialized data */
    .data ALIGN(4) : {
        *(.data*)
    }

    /* Uninitialized data (BSS) */
    .bss ALIGN(4) : {
        *(COMMON)
        *(.bss*)
    }
}

