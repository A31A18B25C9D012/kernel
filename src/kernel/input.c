#include "types.h"

static uint8_t keyboard_buffer[256];
static uint8_t keyboard_head = 0;
static uint8_t keyboard_tail = 0;

static uint32_t mouse_x = 0;
static uint32_t mouse_y = 0;
static uint8_t mouse_buttons = 0;

extern void gui_handle_click(uint32_t x, uint32_t y);

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void keyboard_init(void) {
    keyboard_head = 0;
    keyboard_tail = 0;
}

uint8_t keyboard_read(void) {
    if (keyboard_head == keyboard_tail) {
        return 0;
    }
    uint8_t key = keyboard_buffer[keyboard_tail];
    keyboard_tail = (keyboard_tail + 1) % 256;
    return key;
}

void keyboard_handle(void) {
    if (!(inb(0x64) & 1)) return;
    
    uint8_t scancode = inb(0x60);
    
    if (scancode & 0x80) {
        return;
    }
    
    static const char scancode_map[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
        '*', 0, ' '
    };
    
    if (scancode < sizeof(scancode_map)) {
        char c = scancode_map[scancode];
        if (c) {
            keyboard_buffer[keyboard_head] = c;
            keyboard_head = (keyboard_head + 1) % 256;
        }
    }
}

void mouse_init(void) {
    outb(0x64, 0xA8);
    outb(0x64, 0x20);
    uint8_t status = inb(0x60) | 2;
    outb(0x64, 0x60);
    outb(0x60, status);
    
    outb(0x64, 0xD4);
    outb(0x60, 0xF4);
    inb(0x60);
    
    mouse_x = FB_WIDTH / 2;
    mouse_y = FB_HEIGHT / 2;
    mouse_buttons = 0;
}

void mouse_handle(void) {
    static int packet_index = 0;
    static uint8_t packet[3];
    
    if (!(inb(0x64) & 0x20)) return;
    
    uint8_t data = inb(0x60);
    
    packet[packet_index++] = data;
    if (packet_index == 3) {
        packet_index = 0;
        
        int dx = packet[1];
        int dy = packet[2];
        
        if (packet[0] & 0x10) dx |= 0xFFFFFF00;
        if (packet[0] & 0x20) dy |= 0xFFFFFF00;
        
        int new_x = (int)mouse_x + dx;
        int new_y = (int)mouse_y - dy;
        
        if (new_x < 0) new_x = 0;
        if (new_x >= (int)FB_WIDTH) new_x = FB_WIDTH - 1;
        if (new_y < 0) new_y = 0;
        if (new_y >= (int)FB_HEIGHT) new_y = FB_HEIGHT - 1;
        
        mouse_x = new_x;
        mouse_y = new_y;
        
        uint8_t old_buttons = mouse_buttons;
        mouse_buttons = packet[0] & 0x07;
        
        if ((mouse_buttons & 1) && !(old_buttons & 1)) {
            gui_handle_click(mouse_x, mouse_y);
        }
    }
}

void mouse_get_pos(uint32_t *x, uint32_t *y) {
    *x = mouse_x;
    *y = mouse_y;
}

uint8_t mouse_get_buttons(void) {
    return mouse_buttons;
}