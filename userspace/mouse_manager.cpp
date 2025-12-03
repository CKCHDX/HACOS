#include "mouse_manager.h"

#define MOUSE_DATA_PORT    0x60
#define MOUSE_STATUS_PORT  0x64
#define MOUSE_CMD_PORT     0x64

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

MouseManager::MouseManager() : m_x(640), m_y(360), m_leftPressed(false), m_leftClicked(false) {}

void MouseManager::update() {
    // Simple PS/2 mouse reading (basic implementation)
    static uint8_t mouseBuffer[3] = {0};
    static int bufferIndex = 0;
    
    uint8_t status = inb(MOUSE_STATUS_PORT);
    if ((status & 0x01) == 0) return;  // No data available
    
    uint8_t data = inb(MOUSE_DATA_PORT);
    
    if (bufferIndex == 0) {
        // First byte - check if valid packet start
        if ((data & 0x08) == 0) return;  // Invalid packet
    }
    
    mouseBuffer[bufferIndex] = data;
    bufferIndex++;
    
    if (bufferIndex >= 3) {
        bufferIndex = 0;
        
        // Parse mouse packet
        uint8_t buttons = mouseBuffer[0];
        int dx = mouseBuffer[1];
        int dy = mouseBuffer[2];
        
        // Handle sign extension
        if (buttons & 0x10) dx |= 0xFFFFFF00;
        if (buttons & 0x20) dy |= 0xFFFFFF00;
        
        // Update position
        m_x += dx;
        m_y -= dy;  // Invert Y
        
        // Clamp to screen
        if (m_x < 0) m_x = 0;
        if (m_y < 0) m_y = 0;
        if (m_x > 1280) m_x = 1280;
        if (m_y > 720) m_y = 720;
        
        // Check left button
        bool newLeftPressed = (buttons & 0x01) != 0;
        if (newLeftPressed && !m_leftPressed) {
            m_leftClicked = true;
        }
        m_leftPressed = newLeftPressed;
    }
}
