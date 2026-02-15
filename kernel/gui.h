// kernel/gui.h
// GUI system - desktop, windows, taskbar

#ifndef GUI_H
#define GUI_H

#include "types.h"
#include "graphics.h"

// Maximum windows
#define MAX_WINDOWS 16
#define MAX_DESKTOP_ICONS 24

// Window flags
#define WINDOW_FLAG_VISIBLE    (1 << 0)
#define WINDOW_FLAG_FOCUSED    (1 << 1)
#define WINDOW_FLAG_DRAGGING   (1 << 2)
#define WINDOW_FLAG_RESIZABLE  (1 << 3)
#define WINDOW_FLAG_CLOSABLE   (1 << 4)
#define WINDOW_FLAG_MINIMIZED  (1 << 5)

// Forward declaration
struct window;
typedef struct window window_t;

// Window structure
struct window {
    i32 x, y;               // Position
    u32 width, height;      // Size
    char title[64];         // Window title
    u32 flags;              // Window state flags
    void (*on_draw)(window_t* self);  // Custom draw callback
    void (*on_click)(window_t* self, i32 x, i32 y); // Click handler
    void* user_data;        // Custom data
};

// Desktop icon
typedef struct {
    i32 x, y;               // Position
    char name[32];          // Icon label
    u8 icon_type;           // Icon type (0=folder, 1=file, 2=app, etc.)
    bool selected;          // Is selected
    void (*on_open)(void);  // Double-click handler
} desktop_icon_t;

// Initialize GUI system
void gui_init(void);

// Main GUI loop (call repeatedly)
void gui_update(void);

// Window management
window_t* gui_create_window(const char* title, i32 x, i32 y, u32 width, u32 height);
void gui_destroy_window(window_t* win);
void gui_focus_window(window_t* win);
void gui_show_window(window_t* win);
void gui_hide_window(window_t* win);

// Desktop icons
void gui_add_desktop_icon(const char* name, u8 icon_type, i32 x, i32 y, void (*on_open)(void));

// Start menu
void gui_toggle_start_menu(void);

// Drawing helpers
void gui_draw_button(i32 x, i32 y, u32 w, u32 h, const char* text, bool pressed, bool hover);
void gui_draw_window_controls(window_t* win);

#endif // GUI_H
