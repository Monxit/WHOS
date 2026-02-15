// kernel/graphics.c
// Framebuffer graphics implementation using VESA/VBE via Multiboot

#include "graphics.h"
#include "font.h"
#include "string.h"

// Multiboot info structure (partial - just what we need)
struct multiboot_info {
    u32 flags;
    u32 mem_lower;
    u32 mem_upper;
    u32 boot_device;
    u32 cmdline;
    u32 mods_count;
    u32 mods_addr;
    u32 syms[4];
    u32 mmap_length;
    u32 mmap_addr;
    u32 drives_length;
    u32 drives_addr;
    u32 config_table;
    u32 boot_loader_name;
    u32 apm_table;
    u32 vbe_control_info;
    u32 vbe_mode_info;
    u16 vbe_mode;
    u16 vbe_interface_seg;
    u16 vbe_interface_off;
    u16 vbe_interface_len;
    u64 framebuffer_addr;
    u32 framebuffer_pitch;
    u32 framebuffer_width;
    u32 framebuffer_height;
    u8  framebuffer_bpp;
    u8  framebuffer_type;
    u8  color_info[6];
} __attribute__((packed));

// Framebuffer info
static u32* framebuffer = NULL;
static u32* backbuffer = NULL;
static u32 fb_width = 800;
static u32 fb_height = 600;
static u32 fb_pitch = 0;
static u8  fb_bpp = 32;

// Back buffer allocation (static for simplicity)
#define MAX_BACKBUFFER_SIZE (1920 * 1080 * 4)
static u32 backbuffer_data[MAX_BACKBUFFER_SIZE / 4] __attribute__((aligned(4)));

void graphics_init(u32* multiboot_info) {
    struct multiboot_info* mb = (struct multiboot_info*)multiboot_info;
    
    // Check if framebuffer info is available (flag bit 12)
    if (mb->flags & (1 << 12)) {
        framebuffer = (u32*)(u32)mb->framebuffer_addr;
        fb_width = mb->framebuffer_width;
        fb_height = mb->framebuffer_height;
        fb_pitch = mb->framebuffer_pitch;
        fb_bpp = mb->framebuffer_bpp;
    } else {
        // Fallback: assume standard VGA text mode address (won't work for GUI)
        // In practice, GRUB should provide framebuffer if requested
        framebuffer = (u32*)0xB8000;
        fb_width = 80;
        fb_height = 25;
        fb_pitch = 160;
        fb_bpp = 16;
    }
    
    // Use our static backbuffer
    backbuffer = backbuffer_data;
    
    // Clear to black
    graphics_clear(COLOR_BLACK);
}

u32 graphics_get_width(void) {
    return fb_width;
}

u32 graphics_get_height(void) {
    return fb_height;
}

void graphics_put_pixel(u32 x, u32 y, color_t color) {
    if (x >= fb_width || y >= fb_height) return;
    backbuffer[y * fb_width + x] = color;
}

color_t graphics_get_pixel(u32 x, u32 y) {
    if (x >= fb_width || y >= fb_height) return 0;
    return backbuffer[y * fb_width + x];
}

void graphics_fill_rect(u32 x, u32 y, u32 width, u32 height, color_t color) {
    for (u32 py = y; py < y + height && py < fb_height; py++) {
        for (u32 px = x; px < x + width && px < fb_width; px++) {
            backbuffer[py * fb_width + px] = color;
        }
    }
}

void graphics_draw_rect(u32 x, u32 y, u32 width, u32 height, color_t color) {
    // Top and bottom edges
    for (u32 px = x; px < x + width && px < fb_width; px++) {
        if (y < fb_height) backbuffer[y * fb_width + px] = color;
        if (y + height - 1 < fb_height) backbuffer[(y + height - 1) * fb_width + px] = color;
    }
    // Left and right edges
    for (u32 py = y; py < y + height && py < fb_height; py++) {
        if (x < fb_width) backbuffer[py * fb_width + x] = color;
        if (x + width - 1 < fb_width) backbuffer[py * fb_width + (x + width - 1)] = color;
    }
}

void graphics_draw_line(i32 x0, i32 y0, i32 x1, i32 y1, color_t color) {
    // Bresenham's line algorithm
    i32 dx = x1 - x0;
    i32 dy = y1 - y0;
    i32 sx = (dx > 0) ? 1 : -1;
    i32 sy = (dy > 0) ? 1 : -1;
    
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    
    i32 err = dx - dy;
    
    while (1) {
        if (x0 >= 0 && x0 < (i32)fb_width && y0 >= 0 && y0 < (i32)fb_height) {
            backbuffer[y0 * fb_width + x0] = color;
        }
        
        if (x0 == x1 && y0 == y1) break;
        
        i32 e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

// Helper to interpolate colors for gradient
static color_t lerp_color(color_t c1, color_t c2, u32 t, u32 max) {
    u8 r1 = (c1 >> 16) & 0xFF;
    u8 g1 = (c1 >> 8) & 0xFF;
    u8 b1 = c1 & 0xFF;
    
    u8 r2 = (c2 >> 16) & 0xFF;
    u8 g2 = (c2 >> 8) & 0xFF;
    u8 b2 = c2 & 0xFF;
    
    u8 r = r1 + ((r2 - r1) * t) / max;
    u8 g = g1 + ((g2 - g1) * t) / max;
    u8 b = b1 + ((b2 - b1) * t) / max;
    
    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

void graphics_fill_gradient_v(u32 x, u32 y, u32 width, u32 height,
                              color_t color_top, color_t color_bottom) {
    for (u32 py = 0; py < height && (y + py) < fb_height; py++) {
        color_t row_color = lerp_color(color_top, color_bottom, py, height);
        for (u32 px = x; px < x + width && px < fb_width; px++) {
            backbuffer[(y + py) * fb_width + px] = row_color;
        }
    }
}

void graphics_draw_char(u32 x, u32 y, char c, color_t fg, color_t bg) {
    const u8* glyph = font_get_char(c);
    
    for (u32 py = 0; py < FONT_HEIGHT; py++) {
        u8 row = glyph[py];
        for (u32 px = 0; px < FONT_WIDTH; px++) {
            color_t color = (row & (0x80 >> px)) ? fg : bg;
            if (x + px < fb_width && y + py < fb_height) {
                backbuffer[(y + py) * fb_width + (x + px)] = color;
            }
        }
    }
}

void graphics_draw_string(u32 x, u32 y, const char* str, color_t fg, color_t bg) {
    u32 cx = x;
    while (*str) {
        if (*str == '\n') {
            y += FONT_HEIGHT;
            cx = x;
        } else {
            graphics_draw_char(cx, y, *str, fg, bg);
            cx += FONT_WIDTH;
        }
        str++;
    }
}

void graphics_draw_string_transparent(u32 x, u32 y, const char* str, color_t fg) {
    u32 cx = x;
    while (*str) {
        if (*str == '\n') {
            y += FONT_HEIGHT;
            cx = x;
        } else {
            const u8* glyph = font_get_char(*str);
            for (u32 py = 0; py < FONT_HEIGHT; py++) {
                u8 row = glyph[py];
                for (u32 px = 0; px < FONT_WIDTH; px++) {
                    if (row & (0x80 >> px)) {
                        if (cx + px < fb_width && y + py < fb_height) {
                            backbuffer[(y + py) * fb_width + (cx + px)] = fg;
                        }
                    }
                }
            }
            cx += FONT_WIDTH;
        }
        str++;
    }
}

void graphics_swap_buffers(void) {
    // Copy backbuffer to framebuffer
    // Note: fb_pitch may not equal fb_width * 4 due to alignment
    if (fb_pitch == fb_width * 4) {
        memcpy(framebuffer, backbuffer, fb_width * fb_height * 4);
    } else {
        // Copy row by row if pitch differs
        for (u32 y = 0; y < fb_height; y++) {
            memcpy((u8*)framebuffer + y * fb_pitch, 
                   backbuffer + y * fb_width,
                   fb_width * 4);
        }
    }
}

void graphics_clear(color_t color) {
    for (u32 i = 0; i < fb_width * fb_height; i++) {
        backbuffer[i] = color;
    }
}
