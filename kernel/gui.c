// kernel/gui.c
// GUI system implementation - Windows 7-style desktop

#include "gui.h"
#include "graphics.h"
#include "mouse.h"
#include "keyboard.h"
#include "string.h"
#include "font.h"

// Taskbar dimensions
#define TASKBAR_HEIGHT 40

// Window title bar height
#define TITLE_BAR_HEIGHT 30

// Desktop icon dimensions
#define ICON_WIDTH 64
#define ICON_HEIGHT 80
#define ICON_SPACING 20

// Window storage
static window_t windows[MAX_WINDOWS];
static u32 window_count = 0;
static window_t* focused_window = NULL;
static window_t* dragging_window = NULL;
static i32 drag_offset_x = 0;
static i32 drag_offset_y = 0;

// Desktop icons
static desktop_icon_t desktop_icons[MAX_DESKTOP_ICONS];
static u32 icon_count = 0;

// Start menu state
static bool start_menu_open = false;

// Previous mouse state for click detection
static bool prev_left_button = false;
static i32 prev_mouse_x = 0;
static i32 prev_mouse_y = 0;

// Screen dimensions
static u32 screen_w = 800;
static u32 screen_h = 600;

// Mouse cursor (16x16 bitmap - arrow)
static const u8 cursor_bitmap[16][16] = {
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0},
    {1,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0},
    {1,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0},
    {1,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0},
    {1,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0},
    {1,2,2,2,2,2,2,2,2,1,0,0,0,0,0,0},
    {1,2,2,2,2,2,1,1,1,1,1,0,0,0,0,0},
    {1,2,2,1,2,2,1,0,0,0,0,0,0,0,0,0},
    {1,2,1,0,1,2,2,1,0,0,0,0,0,0,0,0},
    {1,1,0,0,1,2,2,1,0,0,0,0,0,0,0,0},
    {1,0,0,0,0,1,2,2,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0},
};

// Draw mouse cursor
static void draw_cursor(i32 x, i32 y) {
    for (u32 py = 0; py < 16; py++) {
        for (u32 px = 0; px < 16; px++) {
            u8 pixel = cursor_bitmap[py][px];
            if (pixel == 1) {
                graphics_put_pixel(x + px, y + py, COLOR_BLACK);
            } else if (pixel == 2) {
                graphics_put_pixel(x + px, y + py, COLOR_WHITE);
            }
        }
    }
}

// Draw a simple folder icon
static void draw_folder_icon(i32 x, i32 y, bool selected) {
    color_t folder_color = selected ? 0xFF5590D0 : 0xFFE8C34A;
    color_t folder_dark = selected ? 0xFF4080C0 : 0xFFD4A830;
    
    // Folder tab
    graphics_fill_rect(x + 4, y + 8, 16, 6, folder_color);
    // Folder body
    graphics_fill_rect(x + 2, y + 12, 44, 30, folder_color);
    graphics_fill_rect(x + 4, y + 14, 40, 26, folder_dark);
}

// Draw a file icon
static void draw_file_icon(i32 x, i32 y, bool selected) {
    color_t paper = selected ? 0xFFD0D8FF : COLOR_WHITE;
    color_t fold = selected ? 0xFFA0B0E0 : 0xFFE0E0E0;
    
    // Paper
    graphics_fill_rect(x + 10, y + 6, 28, 36, paper);
    graphics_draw_rect(x + 10, y + 6, 28, 36, COLOR_GRAY);
    // Fold corner
    graphics_fill_rect(x + 28, y + 6, 10, 10, fold);
    graphics_draw_line(x + 28, y + 6, x + 28, y + 16, COLOR_GRAY);
    graphics_draw_line(x + 28, y + 16, x + 38, y + 16, COLOR_GRAY);
}

// Draw an app icon
static void draw_app_icon(i32 x, i32 y, bool selected) {
    color_t bg = selected ? 0xFF4080FF : 0xFF2060C0;
    
    graphics_fill_rect(x + 8, y + 8, 32, 32, bg);
    graphics_draw_rect(x + 8, y + 8, 32, 32, COLOR_WHITE);
    // Window lines
    graphics_draw_line(x + 12, y + 16, x + 36, y + 16, COLOR_WHITE);
    graphics_fill_rect(x + 12, y + 20, 24, 16, 0xFF3070D0);
}

// Draw a desktop icon with label
static void draw_desktop_icon(desktop_icon_t* icon) {
    // Draw icon based on type
    switch (icon->icon_type) {
        case 0: draw_folder_icon(icon->x, icon->y, icon->selected); break;
        case 1: draw_file_icon(icon->x, icon->y, icon->selected); break;
        case 2: draw_app_icon(icon->x, icon->y, icon->selected); break;
        default: draw_file_icon(icon->x, icon->y, icon->selected); break;
    }
    
    // Draw label (centered below icon)
    u32 text_len = strlen(icon->name);
    i32 text_x = icon->x + (ICON_WIDTH - text_len * FONT_WIDTH) / 2;
    i32 text_y = icon->y + 48;
    
    if (icon->selected) {
        graphics_fill_rect(text_x - 2, text_y - 1, text_len * FONT_WIDTH + 4, FONT_HEIGHT + 2, COLOR_ACCENT);
    }
    graphics_draw_string_transparent(text_x, text_y, icon->name, COLOR_WHITE);
}

// Draw the desktop background
static void draw_desktop(void) {
    // Gradient background (Windows 7 style)
    graphics_fill_gradient_v(0, 0, screen_w, screen_h - TASKBAR_HEIGHT,
                            0xFF0A4A7C, 0xFF041428);
}

// Draw the taskbar
static void draw_taskbar(void) {
    u32 taskbar_y = screen_h - TASKBAR_HEIGHT;
    
    // Taskbar background (semi-transparent dark)
    graphics_fill_rect(0, taskbar_y, screen_w, TASKBAR_HEIGHT, 0xE0202020);
    
    // Top border (subtle highlight)
    graphics_fill_rect(0, taskbar_y, screen_w, 1, 0xFF404040);
    
    // Start button
    bool start_hover = false;
    mouse_state_t* ms = mouse_get_state();
    if (ms->x >= 4 && ms->x < 60 && ms->y >= (i32)taskbar_y + 4 && ms->y < (i32)taskbar_y + 36) {
        start_hover = true;
    }
    
    color_t start_bg = start_menu_open ? 0xFF3080D0 : (start_hover ? 0xFF404040 : 0xFF303030);
    graphics_fill_rect(4, taskbar_y + 4, 56, 32, start_bg);
    graphics_draw_rect(4, taskbar_y + 4, 56, 32, 0xFF505050);
    graphics_draw_string_transparent(12, taskbar_y + 12, "Start", COLOR_WHITE);
    
    // Clock (right side)
    graphics_draw_string_transparent(screen_w - 60, taskbar_y + 12, "12:00", COLOR_WHITE);
    
    // Draw window buttons in taskbar
    u32 btn_x = 70;
    for (u32 i = 0; i < window_count; i++) {
        if (!(windows[i].flags & WINDOW_FLAG_VISIBLE)) continue;
        
        bool is_focused = (&windows[i] == focused_window);
        color_t btn_bg = is_focused ? 0xFF405080 : 0xFF353535;
        
        graphics_fill_rect(btn_x, taskbar_y + 6, 120, 28, btn_bg);
        graphics_draw_string_transparent(btn_x + 8, taskbar_y + 12, windows[i].title, COLOR_WHITE);
        
        btn_x += 130;
    }
}

// Draw start menu
static void draw_start_menu(void) {
    if (!start_menu_open) return;
    
    u32 menu_x = 0;
    u32 menu_y = screen_h - TASKBAR_HEIGHT - 400;
    u32 menu_w = 300;
    u32 menu_h = 400;
    
    // Menu background
    graphics_fill_rect(menu_x, menu_y, menu_w, menu_h, 0xF0202020);
    graphics_draw_rect(menu_x, menu_y, menu_w, menu_h, 0xFF404040);
    
    // User area at top
    graphics_fill_rect(menu_x, menu_y, menu_w, 60, 0xFF303050);
    graphics_draw_string_transparent(menu_x + 70, menu_y + 22, "User", COLOR_WHITE);
    
    // Menu items
    const char* items[] = {"Documents", "Pictures", "Music", "Computer", "Control Panel", "Devices", "Shut Down"};
    u32 item_count = 7;
    
    mouse_state_t* ms = mouse_get_state();
    
    for (u32 i = 0; i < item_count; i++) {
        u32 item_y = menu_y + 70 + i * 40;
        
        // Hover effect
        bool hover = (ms->x >= (i32)menu_x && ms->x < (i32)(menu_x + menu_w) &&
                     ms->y >= (i32)item_y && ms->y < (i32)(item_y + 36));
        
        if (hover) {
            graphics_fill_rect(menu_x + 4, item_y, menu_w - 8, 36, 0xFF405080);
        }
        
        graphics_draw_string_transparent(menu_x + 50, item_y + 10, items[i], COLOR_WHITE);
    }
}

// Draw a window
static void draw_window(window_t* win) {
    if (!(win->flags & WINDOW_FLAG_VISIBLE)) return;
    if (win->flags & WINDOW_FLAG_MINIMIZED) return;
    
    bool is_focused = (win == focused_window);
    
    // Window shadow
    graphics_fill_rect(win->x + 4, win->y + 4, win->width, win->height, 0x40000000);
    
    // Window background
    graphics_fill_rect(win->x, win->y, win->width, win->height, COLOR_WINDOW_BG);
    
    // Title bar
    color_t title_color = is_focused ? COLOR_TITLE_BAR : 0xFF808080;
    graphics_fill_gradient_v(win->x, win->y, win->width, TITLE_BAR_HEIGHT,
                            title_color, is_focused ? 0xFF3670A0 : 0xFF606060);
    
    // Title text
    graphics_draw_string_transparent(win->x + 10, win->y + 8, win->title, COLOR_WHITE);
    
    // Window controls (close button)
    i32 close_x = win->x + win->width - 46;
    i32 close_y = win->y + 1;
    
    mouse_state_t* ms = mouse_get_state();
    bool close_hover = (ms->x >= close_x && ms->x < close_x + 45 &&
                       ms->y >= close_y && ms->y < close_y + 28);
    
    graphics_fill_rect(close_x, close_y, 45, 28, close_hover ? 0xFFE81123 : 0x00000000);
    graphics_draw_string_transparent(close_x + 18, close_y + 6, "X", COLOR_WHITE);
    
    // Minimize button
    i32 min_x = close_x - 46;
    bool min_hover = (ms->x >= min_x && ms->x < min_x + 45 &&
                     ms->y >= close_y && ms->y < close_y + 28);
    if (min_hover) {
        graphics_fill_rect(min_x, close_y, 45, 28, 0xFF505050);
    }
    graphics_draw_string_transparent(min_x + 18, close_y + 6, "_", COLOR_WHITE);
    
    // Window border
    graphics_draw_rect(win->x, win->y, win->width, win->height, 0xFF404040);
    
    // Custom content draw callback
    if (win->on_draw) {
        win->on_draw(win);
    }
}

// Check if point is in rectangle
static bool point_in_rect(i32 px, i32 py, i32 rx, i32 ry, u32 rw, u32 rh) {
    return px >= rx && px < rx + (i32)rw && py >= ry && py < ry + (i32)rh;
}

// Handle mouse input
static void handle_mouse(void) {
    mouse_state_t* ms = mouse_get_state();
    bool clicked = (ms->left_button && !prev_left_button);
    bool released = (!ms->left_button && prev_left_button);
    
    // Handle window dragging
    if (dragging_window && ms->left_button) {
        dragging_window->x = ms->x - drag_offset_x;
        dragging_window->y = ms->y - drag_offset_y;
        
        // Clamp to screen
        if (dragging_window->y < 0) dragging_window->y = 0;
        if (dragging_window->y > (i32)(screen_h - TASKBAR_HEIGHT - TITLE_BAR_HEIGHT)) {
            dragging_window->y = screen_h - TASKBAR_HEIGHT - TITLE_BAR_HEIGHT;
        }
    }
    
    if (released) {
        dragging_window = NULL;
    }
    
    if (clicked) {
        // Check start button
        u32 taskbar_y = screen_h - TASKBAR_HEIGHT;
        if (point_in_rect(ms->x, ms->y, 4, taskbar_y + 4, 56, 32)) {
            start_menu_open = !start_menu_open;
        }
        // Close start menu if clicking elsewhere
        else if (start_menu_open && !point_in_rect(ms->x, ms->y, 0, screen_h - TASKBAR_HEIGHT - 400, 300, 400)) {
            start_menu_open = false;
        }
        
        // Check window interactions (reverse order for z-order)
        for (i32 i = window_count - 1; i >= 0; i--) {
            window_t* win = &windows[i];
            if (!(win->flags & WINDOW_FLAG_VISIBLE)) continue;
            if (win->flags & WINDOW_FLAG_MINIMIZED) continue;
            
            // Check close button
            i32 close_x = win->x + win->width - 46;
            if (point_in_rect(ms->x, ms->y, close_x, win->y + 1, 45, 28)) {
                win->flags &= ~WINDOW_FLAG_VISIBLE;
                if (focused_window == win) focused_window = NULL;
                break;
            }
            
            // Check minimize button
            i32 min_x = close_x - 46;
            if (point_in_rect(ms->x, ms->y, min_x, win->y + 1, 45, 28)) {
                win->flags |= WINDOW_FLAG_MINIMIZED;
                break;
            }
            
            // Check title bar (start drag)
            if (point_in_rect(ms->x, ms->y, win->x, win->y, win->width, TITLE_BAR_HEIGHT)) {
                gui_focus_window(win);
                dragging_window = win;
                drag_offset_x = ms->x - win->x;
                drag_offset_y = ms->y - win->y;
                break;
            }
            
            // Check window body
            if (point_in_rect(ms->x, ms->y, win->x, win->y, win->width, win->height)) {
                gui_focus_window(win);
                if (win->on_click) {
                    win->on_click(win, ms->x - win->x, ms->y - win->y);
                }
                break;
            }
        }
        
        // Check desktop icons (single-click open for now)
        for (u32 i = 0; i < icon_count; i++) {
            desktop_icon_t* icon = &desktop_icons[i];
            if (point_in_rect(ms->x, ms->y, icon->x, icon->y, ICON_WIDTH, ICON_HEIGHT)) {
                // Deselect others
                for (u32 j = 0; j < icon_count; j++) {
                    desktop_icons[j].selected = false;
                }
                icon->selected = true;

                // Open on click if handler exists
                if (icon->on_open) {
                    icon->on_open();
                }
                break;
            }
        }
        
        // Check taskbar window buttons
        u32 btn_x = 70;
        for (u32 i = 0; i < window_count; i++) {
            if (!(windows[i].flags & WINDOW_FLAG_VISIBLE)) continue;
            
            if (point_in_rect(ms->x, ms->y, btn_x, taskbar_y + 6, 120, 28)) {
                if (windows[i].flags & WINDOW_FLAG_MINIMIZED) {
                    windows[i].flags &= ~WINDOW_FLAG_MINIMIZED;
                }
                gui_focus_window(&windows[i]);
                break;
            }
            btn_x += 130;
        }
    }
    
    prev_left_button = ms->left_button;
    prev_mouse_x = ms->x;
    prev_mouse_y = ms->y;
}

// Public functions

void gui_init(void) {
    screen_w = graphics_get_width();
    screen_h = graphics_get_height();
    
    // Initialize mouse bounds
    mouse_set_bounds(screen_w, screen_h);
    
    // Clear window storage
    for (u32 i = 0; i < MAX_WINDOWS; i++) {
        windows[i].flags = 0;
    }
    window_count = 0;
    icon_count = 0;
}

void gui_update(void) {
    // Handle input
    handle_mouse();
    
    // Draw everything
    draw_desktop();
    
    // Draw desktop icons
    for (u32 i = 0; i < icon_count; i++) {
        draw_desktop_icon(&desktop_icons[i]);
    }
    
    // Draw windows (in order for z-ordering)
    for (u32 i = 0; i < window_count; i++) {
        if (&windows[i] != focused_window) {
            draw_window(&windows[i]);
        }
    }
    // Draw focused window last (on top)
    if (focused_window) {
        draw_window(focused_window);
    }
    
    // Draw taskbar (always on top of windows)
    draw_taskbar();
    
    // Draw start menu
    draw_start_menu();
    
    // Draw mouse cursor (always on top)
    mouse_state_t* ms = mouse_get_state();
    draw_cursor(ms->x, ms->y);
    
    // Swap buffers
    graphics_swap_buffers();
}

window_t* gui_create_window(const char* title, i32 x, i32 y, u32 width, u32 height) {
    if (window_count >= MAX_WINDOWS) return NULL;
    
    window_t* win = &windows[window_count++];
    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    win->flags = WINDOW_FLAG_VISIBLE | WINDOW_FLAG_CLOSABLE;
    win->on_draw = NULL;
    win->on_click = NULL;
    win->user_data = NULL;
    
    // Copy title
    u32 i;
    for (i = 0; i < 63 && title[i]; i++) {
        win->title[i] = title[i];
    }
    win->title[i] = '\0';
    
    // Ensure window stays within screen bounds
    if (win->x < 0) win->x = 0;
    if (win->y < 0) win->y = 0;
    i32 max_x = (i32)screen_w - (i32)win->width;
    i32 max_y = (i32)screen_h - TASKBAR_HEIGHT - (i32)win->height;
    if (win->x > max_x) win->x = max_x;
    if (win->y > max_y) win->y = max_y;
    
    gui_focus_window(win);
    return win;
}

void gui_destroy_window(window_t* win) {
    win->flags = 0;
    if (focused_window == win) focused_window = NULL;
}

void gui_focus_window(window_t* win) {
    if (focused_window) {
        focused_window->flags &= ~WINDOW_FLAG_FOCUSED;
    }
    win->flags |= WINDOW_FLAG_FOCUSED;
    focused_window = win;
    
    // Move to end of array for z-ordering (simple approach)
    // Find window index
    u32 idx = 0;
    for (u32 i = 0; i < window_count; i++) {
        if (&windows[i] == win) {
            idx = i;
            break;
        }
    }
    
    // Shift windows
    window_t temp = windows[idx];
    for (u32 i = idx; i < window_count - 1; i++) {
        windows[i] = windows[i + 1];
    }
    windows[window_count - 1] = temp;
    focused_window = &windows[window_count - 1];
}

void gui_show_window(window_t* win) {
    win->flags |= WINDOW_FLAG_VISIBLE;
}

void gui_hide_window(window_t* win) {
    win->flags &= ~WINDOW_FLAG_VISIBLE;
}

void gui_add_desktop_icon(const char* name, u8 icon_type, i32 x, i32 y, void (*on_open)(void)) {
    if (icon_count >= MAX_DESKTOP_ICONS) return;
    
    desktop_icon_t* icon = &desktop_icons[icon_count++];
    icon->x = x;
    icon->y = y;
    icon->icon_type = icon_type;
    icon->selected = false;
    icon->on_open = on_open;
    
    u32 i;
    for (i = 0; i < 31 && name[i]; i++) {
        icon->name[i] = name[i];
    }
    icon->name[i] = '\0';
}

void gui_toggle_start_menu(void) {
    start_menu_open = !start_menu_open;
}

void gui_draw_button(i32 x, i32 y, u32 w, u32 h, const char* text, bool pressed, bool hover) {
    color_t bg = pressed ? 0xFFCCE4F7 : (hover ? 0xFFE5F1FB : COLOR_BUTTON_BG);
    color_t border = pressed || hover ? COLOR_ACCENT : 0xFFADADAD;
    
    graphics_fill_rect(x, y, w, h, bg);
    graphics_draw_rect(x, y, w, h, border);
    
    // Center text
    u32 text_len = strlen(text);
    i32 tx = x + (w - text_len * FONT_WIDTH) / 2;
    i32 ty = y + (h - FONT_HEIGHT) / 2;
    graphics_draw_string_transparent(tx, ty, text, COLOR_BLACK);
}
