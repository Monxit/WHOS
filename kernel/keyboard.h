// kernel/keyboard.h
// PS/2 Keyboard driver

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

// Key state structure
typedef struct {
    bool shift_pressed;
    bool ctrl_pressed;
    bool alt_pressed;
    bool caps_lock;
} keyboard_state_t;

// Initialize keyboard driver
void keyboard_init(void);

// Get the last key pressed (0 if none)
char keyboard_get_char(void);

// Check if a key is available
bool keyboard_has_key(void);

// Get current keyboard state
keyboard_state_t* keyboard_get_state(void);

#endif // KEYBOARD_H
