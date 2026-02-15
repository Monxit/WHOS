// kernel/graphics.h
// Framebuffer graphics driver

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "types.h"

// RGB color (32-bit ARGB format)
typedef u32 color_t;

// Predefined colors
#define COLOR_BLACK       0xFF000000
#define COLOR_WHITE       0xFFFFFFFF
#define COLOR_RED         0xFFFF0000
#define COLOR_GREEN       0xFF00FF00
#define COLOR_BLUE        0xFF0000FF
#define COLOR_YELLOW      0xFFFFFF00
#define COLOR_CYAN        0xFF00FFFF
#define COLOR_MAGENTA     0xFFFF00FF
#define COLOR_GRAY        0xFF808080
#define COLOR_DARK_GRAY   0xFF404040
#define COLOR_LIGHT_GRAY  0xFFC0C0C0

// Windows 7-ish colors
#define COLOR_DESKTOP_BG  0xFF1E4176  // Blue gradient start
#define COLOR_TASKBAR_BG  0xCC1A1A1A  // Dark semi-transparent
#define COLOR_WINDOW_BG   0xFFF0F0F0  // Light gray window
#define COLOR_TITLE_BAR   0xFF4A90C2  // Window title bar blue
#define COLOR_TITLE_TEXT  0xFFFFFFFF
#define COLOR_BUTTON_BG   0xFFE1E1E1
#define COLOR_BUTTON_HOVER 0xFFCCE4F7
#define COLOR_ACCENT      0xFF0078D7  // Windows blue accent

// Initialize graphics with multiboot info
void graphics_init(u32* multiboot_info);

// Get screen dimensions
u32 graphics_get_width(void);
u32 graphics_get_height(void);

// Basic drawing primitives
void graphics_put_pixel(u32 x, u32 y, color_t color);
void graphics_fill_rect(u32 x, u32 y, u32 width, u32 height, color_t color);
void graphics_draw_rect(u32 x, u32 y, u32 width, u32 height, color_t color);
void graphics_draw_line(i32 x0, i32 y0, i32 x1, i32 y1, color_t color);

// Gradient fill
void graphics_fill_gradient_v(u32 x, u32 y, u32 width, u32 height, 
                              color_t color_top, color_t color_bottom);

// Text rendering (8x16 bitmap font)
void graphics_draw_char(u32 x, u32 y, char c, color_t fg, color_t bg);
void graphics_draw_string(u32 x, u32 y, const char* str, color_t fg, color_t bg);
void graphics_draw_string_transparent(u32 x, u32 y, const char* str, color_t fg);

// Double buffering
void graphics_swap_buffers(void);
void graphics_clear(color_t color);

// Get/set pixels (for sprites/icons)
color_t graphics_get_pixel(u32 x, u32 y);

#endif // GRAPHICS_H
