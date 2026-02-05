#include "types.h"

static uint32_t *fb = (uint32_t*)FB_BASE;
static uint32_t fb_width = FB_WIDTH;
static uint32_t fb_height = FB_HEIGHT;

void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color);

void fb_init(void) {
    uint16_t *vga = (uint16_t*)0xB8000;
    for (int i = 0; i < 80 * 25; i++) {
        vga[i] = 0;
    }
}

void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= fb_width || y >= fb_height) return;
    fb[y * fb_width + x] = color;
}

void fb_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (uint32_t j = y; j < y + h && j < fb_height; j++) {
        for (uint32_t i = x; i < x + w && i < fb_width; i++) {
            fb[j * fb_width + i] = color;
        }
    }
}

void fb_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (uint32_t i = x; i < x + w && i < fb_width; i++) {
        fb_put_pixel(i, y, color);
        if (y + h > 0) fb_put_pixel(i, y + h - 1, color);
    }
    for (uint32_t j = y; j < y + h && j < fb_height; j++) {
        fb_put_pixel(x, j, color);
        if (x + w > 0) fb_put_pixel(x + w - 1, j, color);
    }
}

void fb_clear(uint32_t color) {
    for (uint32_t i = 0; i < fb_width * fb_height; i++) {
        fb[i] = color;
    }
}

void fb_draw_char(uint32_t x, uint32_t y, char c, uint32_t color);

void fb_draw_text(uint32_t x, uint32_t y, const char *text, uint32_t color) {
    uint32_t cx = x;
    while (*text) {
        if (*text == '\n') {
            cx = x;
            y += 12;
        } else {
            fb_draw_char(cx, y, *text, color);
            cx += 8;
        }
        text++;
    }
}

void fb_wipe(void) {
    volatile uint32_t *vfb = (volatile uint32_t*)fb;
    for (uint32_t i = 0; i < fb_width * fb_height; i++) {
        vfb[i] = 0;
    }
}   