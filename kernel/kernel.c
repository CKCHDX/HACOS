#include <stdint.h>

typedef struct {
    uint32_t version;
    uint32_t horizontal_resolution;
    uint32_t vertical_resolution;
    uint32_t pixel_format;
    struct {
        uint32_t red_mask;
        uint32_t green_mask;
        uint32_t blue_mask;
        uint32_t reserved_mask;
    } pixel_information;
    uint32_t pixels_per_scan_line;
} gop_mode_info_t;

typedef struct {
    uint32_t max_mode;
    uint32_t mode;
    gop_mode_info_t *info;
    uint64_t size_of_info;
    uint64_t framebuffer_base;
    uint64_t framebuffer_size;
} gop_mode_t;

// Port I/O functions
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// PS/2 Keyboard controller
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Scancode to ASCII mapping (US keyboard layout)
static const char scancode_to_ascii[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char scancode_to_ascii_shift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static uint8_t shift_pressed = 0;
static uint64_t g_tick_count = 0;
static int last_key_returned = -999;  // Track what we returned

// C++ entry point
extern void userspace_main(uint32_t* framebuffer, uint32_t width,
                          uint32_t height, uint32_t pitch);

void kernel_main(gop_mode_t *gop_mode) {
    uint32_t *framebuffer = (uint32_t *)gop_mode->framebuffer_base;
    uint32_t width = gop_mode->info->horizontal_resolution;
    uint32_t height = gop_mode->info->vertical_resolution;
    uint32_t pitch = gop_mode->info->pixels_per_scan_line;

    userspace_main(framebuffer, width, height, pitch);

    while(1) {
        __asm__("hlt");
    }
}

// FIXED: Return key ONCE, then return -1 until released
int get_key_async() {
    // Check if keyboard has data
    uint8_t status = inb(KEYBOARD_STATUS_PORT);
    if ((status & 0x01) == 0) {
        return -1;  // No key available
    }

    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    // Handle shift keys
    if (scancode == 0x2A || scancode == 0x36) {  // Left/Right Shift pressed
        shift_pressed = 1;
        return -1;
    }

    if (scancode == 0xAA || scancode == 0xB6) {  // Left/Right Shift released
        shift_pressed = 0;
        return -1;
    }

    // Ignore key releases (scancode >= 0x80)
    if (scancode >= 0x80) {
        return -1;
    }

    // Convert scancode to ASCII
    char c;
    if (shift_pressed) {
        c = scancode_to_ascii_shift[scancode];
    } else {
        c = scancode_to_ascii[scancode];
    }

    if (c == 0) {
        return -1;
    }

    return (int)c;
}

void delay_ms(int ms) {
    // CPU-based delay (approximately ms milliseconds)
    for (volatile uint64_t i = 0; i < (uint64_t)ms * 500000; i++) {
        __asm__ volatile ("nop");
    }
}

uint64_t get_ticks() {
    return g_tick_count;
}

void tick_increment() {
    g_tick_count++;
}