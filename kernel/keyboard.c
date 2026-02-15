// kernel/keyboard.c
// PS/2 Keyboard driver implementation

#include "keyboard.h"
#include "idt.h"
#include "io.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Circular buffer for key presses
#define KEY_BUFFER_SIZE 256
static char key_buffer[KEY_BUFFER_SIZE];
static u32 buffer_start = 0;
static u32 buffer_end = 0;

// Keyboard state
static keyboard_state_t kb_state = {false, false, false, false};

// US QWERTY scancode to ASCII (set 1, lowercase)
static const char scancode_to_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,  '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0,  ' ', 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Shifted characters
static const char scancode_to_ascii_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,  '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0,  ' ', 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static void keyboard_handler(struct registers* regs) {
    (void)regs;
    
    u8 scancode = inb(KEYBOARD_DATA_PORT);
    
    // Key release (bit 7 set)
    if (scancode & 0x80) {
        scancode &= 0x7F;
        // Handle modifier key releases
        if (scancode == 0x2A || scancode == 0x36) { // Left/Right Shift
            kb_state.shift_pressed = false;
        } else if (scancode == 0x1D) { // Ctrl
            kb_state.ctrl_pressed = false;
        } else if (scancode == 0x38) { // Alt
            kb_state.alt_pressed = false;
        }
        return;
    }
    
    // Handle modifier key presses
    if (scancode == 0x2A || scancode == 0x36) { // Left/Right Shift
        kb_state.shift_pressed = true;
        return;
    } else if (scancode == 0x1D) { // Ctrl
        kb_state.ctrl_pressed = true;
        return;
    } else if (scancode == 0x38) { // Alt
        kb_state.alt_pressed = true;
        return;
    } else if (scancode == 0x3A) { // Caps Lock
        kb_state.caps_lock = !kb_state.caps_lock;
        return;
    }
    
    // Convert scancode to ASCII
    char c;
    if (kb_state.shift_pressed) {
        c = scancode_to_ascii_shift[scancode];
    } else {
        c = scancode_to_ascii[scancode];
    }
    
    // Apply caps lock for letters
    if (kb_state.caps_lock && c >= 'a' && c <= 'z') {
        c = c - 'a' + 'A';
    } else if (kb_state.caps_lock && c >= 'A' && c <= 'Z') {
        c = c - 'A' + 'a';
    }
    
    // Add to buffer if valid
    if (c != 0) {
        u32 next_end = (buffer_end + 1) % KEY_BUFFER_SIZE;
        if (next_end != buffer_start) { // Buffer not full
            key_buffer[buffer_end] = c;
            buffer_end = next_end;
        }
    }
}

void keyboard_init(void) {
    // Register keyboard interrupt handler (IRQ1 = INT 33)
    register_interrupt_handler(33, keyboard_handler);
    
    // Enable keyboard IRQ (unmask IRQ1)
    u8 mask = inb(0x21);
    mask &= ~(1 << 1); // Clear bit 1
    outb(0x21, mask);
}

char keyboard_get_char(void) {
    if (buffer_start == buffer_end) {
        return 0; // Buffer empty
    }
    char c = key_buffer[buffer_start];
    buffer_start = (buffer_start + 1) % KEY_BUFFER_SIZE;
    return c;
}

bool keyboard_has_key(void) {
    return buffer_start != buffer_end;
}

keyboard_state_t* keyboard_get_state(void) {
    return &kb_state;
}
