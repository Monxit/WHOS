// kernel/mouse.h
// PS/2 Mouse driver

#ifndef MOUSE_H
#define MOUSE_H

#include "types.h"

// Mouse state structure
typedef struct {
    i32 x;              // Current X position
    i32 y;              // Current Y position
    bool left_button;   // Left button pressed
    bool right_button;  // Right button pressed
    bool middle_button; // Middle button pressed
} mouse_state_t;

// Initialize mouse driver
void mouse_init(void);

// Get current mouse state
mouse_state_t* mouse_get_state(void);

// Set screen bounds for mouse clamping
void mouse_set_bounds(u32 width, u32 height);

#endif // MOUSE_H
