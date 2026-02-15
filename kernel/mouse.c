// kernel/mouse.c
// PS/2 Mouse driver implementation

#include "mouse.h"
#include "idt.h"
#include "io.h"

#define MOUSE_DATA_PORT    0x60
#define MOUSE_STATUS_PORT  0x64
#define MOUSE_COMMAND_PORT 0x64

// Mouse packet state machine
static u8 mouse_cycle = 0;
static i8 mouse_bytes[3];

// Mouse state
static mouse_state_t mouse_state = {400, 300, false, false, false};
static u32 screen_width = 800;
static u32 screen_height = 600;

// Wait for mouse controller to be ready for input
static void mouse_wait(u8 type) {
    u32 timeout = 100000;
    if (type == 0) {
        // Wait for output buffer to be full
        while (timeout--) {
            if (inb(MOUSE_STATUS_PORT) & 1) return;
        }
    } else {
        // Wait for input buffer to be empty
        while (timeout--) {
            if (!(inb(MOUSE_STATUS_PORT) & 2)) return;
        }
    }
}

// Write to mouse
static void mouse_write(u8 data) {
    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, 0xD4); // Tell controller we're writing to mouse
    mouse_wait(1);
    outb(MOUSE_DATA_PORT, data);
}

// Read from mouse
static u8 mouse_read(void) {
    mouse_wait(0);
    return inb(MOUSE_DATA_PORT);
}

static void mouse_handler(struct registers* regs) {
    (void)regs;
    
    u8 status = inb(MOUSE_STATUS_PORT);
    if (!(status & 0x20)) return; // Not a mouse packet
    
    i8 data = (i8)inb(MOUSE_DATA_PORT);
    
    switch (mouse_cycle) {
        case 0:
            mouse_bytes[0] = data;
            // Check if this is a valid first byte (bit 3 should be set)
            if (data & 0x08) {
                mouse_cycle++;
            }
            break;
        case 1:
            mouse_bytes[1] = data;
            mouse_cycle++;
            break;
        case 2:
            mouse_bytes[2] = data;
            mouse_cycle = 0;
            
            // Process complete packet
            // Byte 0: Y overflow, X overflow, Y sign, X sign, 1, Middle, Right, Left
            // Byte 1: X movement
            // Byte 2: Y movement
            
            mouse_state.left_button   = mouse_bytes[0] & 0x01;
            mouse_state.right_button  = mouse_bytes[0] & 0x02;
            mouse_state.middle_button = mouse_bytes[0] & 0x04;
            
            // Update position (Y is inverted in PS/2)
            mouse_state.x += mouse_bytes[1];
            mouse_state.y -= mouse_bytes[2];
            
            // Clamp to screen bounds
            if (mouse_state.x < 0) mouse_state.x = 0;
            if (mouse_state.y < 0) mouse_state.y = 0;
            if (mouse_state.x >= (i32)screen_width)  mouse_state.x = screen_width - 1;
            if (mouse_state.y >= (i32)screen_height) mouse_state.y = screen_height - 1;
            break;
    }
}

void mouse_init(void) {
    // Enable auxiliary device (mouse)
    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, 0xA8);
    
    // Enable interrupts
    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, 0x20); // Get current status
    u8 status = mouse_read();
    status |= 2;  // Enable IRQ12
    status &= ~0x20; // Enable mouse clock
    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, 0x60); // Set status
    mouse_wait(1);
    outb(MOUSE_DATA_PORT, status);
    
    // Use default settings
    mouse_write(0xF6);
    mouse_read(); // ACK
    
    // Enable data reporting
    mouse_write(0xF4);
    mouse_read(); // ACK
    
    // Register mouse interrupt handler (IRQ12 = INT 44)
    register_interrupt_handler(44, mouse_handler);
    
    // Enable mouse IRQ (unmask IRQ12 on slave PIC)
    u8 mask = inb(0xA1);
    mask &= ~(1 << 4); // Clear bit 4 (IRQ12)
    outb(0xA1, mask);
}

mouse_state_t* mouse_get_state(void) {
    return &mouse_state;
}

void mouse_set_bounds(u32 width, u32 height) {
    screen_width = width;
    screen_height = height;
    // Re-center mouse
    mouse_state.x = width / 2;
    mouse_state.y = height / 2;
}
