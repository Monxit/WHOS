// kernel/kernel.c
// Main kernel entry point - initializes system and runs GUI

#include "types.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "mouse.h"
#include "graphics.h"
#include "gui.h"
#include "string.h"
#include "filesystem.h"

// Sample application windows
static void notepad_draw(window_t* win);
static void notepad_click(window_t* win, i32 x, i32 y);

// Notepad text buffer
static char notepad_buffer[1024] = "Welcome to WHOS!\n\nThis is a simple text area.\nType here...";
static u32 notepad_cursor = 0;

static void notepad_draw(window_t* win) {
    // Draw text area background
    graphics_fill_rect(win->x + 5, win->y + 35, win->width - 10, win->height - 40, COLOR_WHITE);
    graphics_draw_rect(win->x + 5, win->y + 35, win->width - 10, win->height - 40, 0xFF808080);
    
    // Draw text
    u32 x = win->x + 10;
    u32 y = win->y + 40;
    u32 line_height = 18;
    
    for (u32 i = 0; notepad_buffer[i] && y < win->y + win->height - 20; i++) {
        if (notepad_buffer[i] == '\n') {
            y += line_height;
            x = win->x + 10;
        } else {
            graphics_draw_char(x, y, notepad_buffer[i], COLOR_BLACK, COLOR_WHITE);
            x += 8;
            if (x > win->x + win->width - 20) {
                y += line_height;
                x = win->x + 10;
            }
        }
    }
}

static void notepad_click(window_t* win, i32 x, i32 y) {
    (void)win; (void)x; (void)y;
    // Could implement click-to-position cursor here
}

// Settings window draw
static void settings_draw(window_t* win) {
    u32 y = win->y + 40;
    
    graphics_draw_string_transparent(win->x + 20, y, "System Settings", COLOR_BLACK);
    y += 30;
    
    // Draw some fake settings
    graphics_draw_string_transparent(win->x + 20, y, "Display", 0xFF0066CC);
    y += 25;
    graphics_draw_string_transparent(win->x + 40, y, "Resolution: 800x600", COLOR_DARK_GRAY);
    y += 20;
    graphics_draw_string_transparent(win->x + 40, y, "Color Depth: 32-bit", COLOR_DARK_GRAY);
    y += 35;
    
    graphics_draw_string_transparent(win->x + 20, y, "System", 0xFF0066CC);
    y += 25;
    graphics_draw_string_transparent(win->x + 40, y, "OS: WHOS 1.0", COLOR_DARK_GRAY);
    y += 20;
    graphics_draw_string_transparent(win->x + 40, y, "Kernel: Custom 32-bit", COLOR_DARK_GRAY);
}

// File Explorer window draw
static void explorer_draw(window_t* win) {
    // Address bar
    graphics_fill_rect(win->x + 5, win->y + 35, win->width - 10, 25, COLOR_WHITE);
    graphics_draw_rect(win->x + 5, win->y + 35, win->width - 10, 25, 0xFF808080);
    graphics_draw_string_transparent(win->x + 10, win->y + 40, "C:\\Users\\User\\Desktop", COLOR_DARK_GRAY);
    
    // Sidebar
    graphics_fill_rect(win->x + 5, win->y + 65, 150, win->height - 75, 0xFFF5F5F5);
    graphics_draw_rect(win->x + 5, win->y + 65, 150, win->height - 75, 0xFFE0E0E0);
    
    // Sidebar items
    graphics_draw_string_transparent(win->x + 15, win->y + 75, "Quick Access", 0xFF0066CC);
    graphics_draw_string_transparent(win->x + 20, win->y + 95, "Desktop", COLOR_BLACK);
    graphics_draw_string_transparent(win->x + 20, win->y + 115, "Documents", COLOR_BLACK);
    graphics_draw_string_transparent(win->x + 20, win->y + 135, "Downloads", COLOR_BLACK);
    graphics_draw_string_transparent(win->x + 20, win->y + 155, "Pictures", COLOR_BLACK);
    
    // Main content area
    graphics_fill_rect(win->x + 160, win->y + 65, win->width - 170, win->height - 75, COLOR_WHITE);
    
    // List files from filesystem
    char* file_list[MAX_FILES];
    u32 file_count = fs_list_files("/", file_list, MAX_FILES);
    
    u32 y_offset = 80;
    if (file_count == 0) {
        graphics_draw_string_transparent(win->x + 180, win->y + y_offset, "[Empty]", COLOR_DARK_GRAY);
    } else {
        for (u32 i = 0; i < file_count && i < 15; i++) {
            graphics_draw_string_transparent(win->x + 180, win->y + y_offset, "[File] ", COLOR_DARK_GRAY);
            graphics_draw_string_transparent(win->x + 230, win->y + y_offset, file_list[i], COLOR_BLACK);
            y_offset += 20;
        }
    }
}

// Open application callbacks for desktop icons
static window_t* notepad_window = NULL;
static window_t* settings_window = NULL;
static window_t* explorer_window = NULL;

static void open_notepad(void) {
    // If window exists and is visible, just focus it
    if (notepad_window && (notepad_window->flags & WINDOW_FLAG_VISIBLE)) {
        // Restore if minimized
        if (notepad_window->flags & WINDOW_FLAG_MINIMIZED) {
            notepad_window->flags &= ~WINDOW_FLAG_MINIMIZED;
        }
        gui_focus_window(notepad_window);
        return;
    }
    
    // Create new window
    notepad_window = gui_create_window("Notepad - Untitled", 80, 60, 640, 480);
    if (notepad_window) {
        notepad_window->on_draw = notepad_draw;
        notepad_window->on_click = notepad_click;
        
        // Initialize with welcome text
        const char* welcome = "Welcome to WHOS Notepad!\n\nType here to create your text file.\nPress F2 to save to desktop.\n\n";
        strcpy(notepad_buffer, welcome);
        notepad_cursor = strlen(notepad_buffer);
    }
}

static void open_settings(void) {
    if (!settings_window || !(settings_window->flags & WINDOW_FLAG_VISIBLE)) {
        settings_window = gui_create_window("Settings", 200, 80, 400, 300);
        if (settings_window) {
            settings_window->on_draw = settings_draw;
        }
    } else {
        gui_focus_window(settings_window);
    }
}

static void open_explorer(void) {
    if (!explorer_window || !(explorer_window->flags & WINDOW_FLAG_VISIBLE)) {
        explorer_window = gui_create_window("File Explorer", 100, 60, 600, 400);
        if (explorer_window) {
            explorer_window->on_draw = explorer_draw;
        }
    } else {
        gui_focus_window(explorer_window);
    }
}

// Main kernel entry point
// Parameter: pointer to multiboot info structure
void kernel_main(u32* multiboot_info) {
    // Initialize GDT (required for proper segment setup)
    gdt_init();
    
    // Initialize IDT and interrupts (for keyboard/mouse)
    idt_init();
    
    // Initialize graphics with framebuffer from multiboot
    graphics_init(multiboot_info);
    
    // Initialize input devices
    keyboard_init();
    mouse_init();
    
    // Initialize GUI system
    gui_init();
    
    // Initialize filesystem
    fs_init();
    
    // Add desktop icons
    gui_add_desktop_icon("My Computer", 2, 20, 20, open_explorer);
    gui_add_desktop_icon("Documents", 0, 20, 110, open_explorer);
    gui_add_desktop_icon("Notepad", 2, 20, 200, open_notepad);
    gui_add_desktop_icon("Settings", 2, 20, 290, open_settings);
    gui_add_desktop_icon("Recycle Bin", 0, 20, 380, NULL);
    
    // Enable interrupts
    __asm__ __volatile__("sti");
    
    // File save counter for unique names
    static u32 file_counter = 1;
    
    // Main GUI loop
    for (;;) {
        // Process keyboard input
        while (keyboard_has_key()) {
            char c = keyboard_get_char();
            
            // Check if notepad is focused for text input
            if (notepad_window && (notepad_window->flags & WINDOW_FLAG_FOCUSED)) {
                // F2 key (scancode might be different, using ESC as alternative)
                if (c == 27) { // ESC key = save file
                    // Create filename
                    char filename[64];
                    filename[0] = 'd';
                    filename[1] = 'o';
                    filename[2] = 'c';
                    filename[3] = '0' + (file_counter / 10);
                    filename[4] = '0' + (file_counter % 10);
                    filename[5] = '.';
                    filename[6] = 't';
                    filename[7] = 'x';
                    filename[8] = 't';
                    filename[9] = '\0';
                    
                    // Save to filesystem
                    file_t* file = fs_create_file(filename);
                    if (file) {
                        fs_write_file(file, (u8*)notepad_buffer, notepad_cursor);
                        file_counter++;
                        
                        // Update window title
                        strcpy(notepad_window->title, "Notepad - ");
                        u32 title_len = strlen(notepad_window->title);
                        for (u32 i = 0; i < 9 && filename[i]; i++) {
                            notepad_window->title[title_len + i] = filename[i];
                        }
                        notepad_window->title[title_len + 9] = '\0';
                    }
                    continue;
                }
                
                // Backspace
                if (c == '\b') {
                    if (notepad_cursor > 0) {
                        notepad_cursor--;
                        notepad_buffer[notepad_cursor] = '\0';
                    }
                }
                // Enter key - add newline
                else if (c == '\n' || c == '\r') {
                    if (notepad_cursor < sizeof(notepad_buffer) - 1) {
                        notepad_buffer[notepad_cursor++] = '\n';
                        notepad_buffer[notepad_cursor] = '\0';
                    }
                }
                // Regular characters
                else if (c >= 32 && c < 127 && notepad_cursor < sizeof(notepad_buffer) - 1) {
                    notepad_buffer[notepad_cursor++] = c;
                    notepad_buffer[notepad_cursor] = '\0';
                }
            }
        }
        
        // Update GUI
        gui_update();
        
        // Small delay to reduce CPU usage
        for (volatile u32 i = 0; i < 100000; i++);
    }
}
