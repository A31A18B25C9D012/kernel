#include "types.h"

extern void cache_init(void);
extern void cache_wipe(void);
extern void xfce_init(void);
extern void xfce_wipe(void);
extern void mem_init(void);
extern void mem_wipe_all(void);
extern void fb_init(void);
extern void fb_wipe(void);
extern void fb_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
extern void fb_draw_text(uint32_t x, uint32_t y, const char *text, uint32_t color);
extern void gui_render(void);
extern void gui_cleanup(void);
extern void gui_create_window(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const char *title);
extern void gui_create_button(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const char *text, void (*cb)(void));
extern void keyboard_init(void);
extern void keyboard_handle(void);
extern uint8_t keyboard_read(void);
extern void mouse_init(void);
extern void mouse_handle(void);
extern void mouse_get_pos(uint32_t *x, uint32_t *y);

static int running = 1;
static char input_buffer[256];
static int input_len = 0;

void halt(void) {
    running = 0;
}

void clear_screen(void) {
    gui_cleanup();
    gui_render();
}

void show_info(void) {
    gui_create_window(220, 170, 380, 220, "info");
    gui_render();
}

void handle_input(void) {
    uint8_t key = keyboard_read();
    if (key == 0) return;
    
    if (key == '\n') {
        if (input_len > 0) {
            if (input_buffer[0] == 'h' && input_buffer[1] == 'a' && 
                input_buffer[2] == 'l' && input_buffer[3] == 't' && input_len == 4) {
                halt();
            } else if (input_buffer[0] == 'c' && input_buffer[1] == 'l' && 
                       input_buffer[2] == 'e' && input_buffer[3] == 'a' && 
                       input_buffer[4] == 'r' && input_len == 5) {
                clear_screen();
            } else if (input_buffer[0] == 'i' && input_buffer[1] == 'n' && 
                       input_buffer[2] == 'f' && input_buffer[3] == 'o' && input_len == 4) {
                show_info();
            }
            input_len = 0;
            for (int i = 0; i < 256; i++) input_buffer[i] = 0;
        }
    } else if (key == '\b') {
        if (input_len > 0) {
            input_len--;
            input_buffer[input_len] = 0;
        }
    } else if (input_len < 255) {
        input_buffer[input_len++] = key;
        input_buffer[input_len] = 0;
    }
}

void draw_cursor(void) {
    uint32_t x, y;
    mouse_get_pos(&x, &y);
    
    fb_fill_rect(x, y, 2, 14, COLOR_FG);
    fb_fill_rect(x, y, 10, 2, COLOR_FG);
}

void __attribute__((section(".text.entry"))) kmain(void) {
    xfce_init();
    cache_init();
    mem_init();
    fb_init();
    keyboard_init();
    mouse_init();
    
    gui_create_window(120, 120, 580, 380, "term");
    gui_create_button(770, 120, 100, 44, "info", show_info);
    gui_create_button(770, 174, 100, 44, "clear", clear_screen);
    gui_create_button(770, 228, 100, 44, "halt", halt);
    
    while (running) {
        keyboard_handle();
        mouse_handle();
        handle_input();
        
        gui_render();
        
        fb_draw_text(140, 170, "> ", COLOR_FG);
        fb_draw_text(156, 170, input_buffer, COLOR_FG);
        
        draw_cursor();
        
        for (volatile int i = 0; i < 100000; i++);
    }
    
    fb_wipe();
    mem_wipe_all();
    xfce_wipe();
    cache_wipe();
    gui_cleanup();
    
    __asm__ volatile (
        "cli\n"
        "hlt\n"
    );
}