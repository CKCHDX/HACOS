#include "renderer.h"

extern "C" {
    // Math helpers

}

Renderer::Renderer(uint32_t* fb, uint32_t width, uint32_t height, uint32_t pitch)
    : m_framebuffer(fb), m_width(width), m_height(height), 
      m_pitch(pitch), m_globalAlpha(255) {}

void Renderer::clear(Color color) {
    uint32_t c = color.toRGBA();
    for (uint32_t i = 0; i < m_width * m_height; i++) {
        m_framebuffer[i] = c;
    }
}

uint32_t Renderer::blend(uint32_t fg, uint32_t bg, uint8_t alpha) {
    if (alpha >= 255) return fg;
    if (alpha == 0) return bg;
    
    uint32_t r1 = (fg >> 16) & 0xFF;
    uint32_t g1 = (fg >> 8) & 0xFF;
    uint32_t b1 = fg & 0xFF;
    
    uint32_t r2 = (bg >> 16) & 0xFF;
    uint32_t g2 = (bg >> 8) & 0xFF;
    uint32_t b2 = bg & 0xFF;
    
    uint32_t r = (r1 * alpha + r2 * (255 - alpha)) / 255;
    uint32_t g = (g1 * alpha + g2 * (255 - alpha)) / 255;
    uint32_t b = (b1 * alpha + b2 * (255 - alpha)) / 255;
    
    return (r << 16) | (g << 8) | b;
}

void Renderer::drawPixel(int x, int y, Color color) {
    if (x < 0 || x >= (int)m_width || y < 0 || y >= (int)m_height) return;
    
    uint32_t offset = y * m_pitch + x;
    if (color.a == 255) {
        m_framebuffer[offset] = color.toRGBA();
    } else {
        m_framebuffer[offset] = blend(color.toRGBA(), m_framebuffer[offset], color.a);
    }
}

int Renderer::distanceSquared(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    return dx * dx + dy * dy;
}

void Renderer::drawCircle(int cx, int cy, int radius, Color color) {
    for (int y = cy - radius - 1; y <= cy + radius + 1; y++) {
        for (int x = cx - radius - 1; x <= cx + radius + 1; x++) {
            int distSq = distanceSquared(cx, cy, x, y);
            int radiusSq = radius * radius;
            
            if (distSq >= radiusSq && distSq <= (radius + 1) * (radius + 1)) {
                drawPixel(x, y, color);
            }
        }
    }
}

void Renderer::drawFilledCircle(int cx, int cy, int radius, Color color) {
    for (int y = cy - radius; y <= cy + radius; y++) {
        for (int x = cx - radius; x <= cx + radius; x++) {
            if (distanceSquared(cx, cy, x, y) <= radius * radius) {
                drawPixel(x, y, color);
            }
        }
    }
}

void Renderer::drawRectangle(int x, int y, int width, int height, Color color) {
    // Top and bottom
    for (int i = 0; i < width; i++) {
        drawPixel(x + i, y, color);
        drawPixel(x + i, y + height - 1, color);
    }
    
    // Left and right
    for (int i = 0; i < height; i++) {
        drawPixel(x, y + i, color);
        drawPixel(x + width - 1, y + i, color);
    }
}

void Renderer::drawFilledRectangle(int x, int y, int width, int height, Color color) {
    uint32_t rgba = color.toRGBA();
    
    for (int py = y; py < y + height && py < (int)m_height; py++) {
        if (py < 0) continue;
        uint32_t* row = m_framebuffer + (py * m_pitch);
        for (int px = x; px < x + width && px < (int)m_width; px++) {
            if (px < 0) continue;
            row[px] = rgba;
        }
    }
}


void Renderer::drawRoundedRect(int x, int y, int width, int height, int radius, Color color) {
    // Main body
    drawFilledRectangle(x + radius, y, width - 2 * radius, height, color);
    drawFilledRectangle(x, y + radius, width, height - 2 * radius, color);
    
    // Four corners
    drawFilledCircle(x + radius, y + radius, radius, color);
    drawFilledCircle(x + width - radius, y + radius, radius, color);
    drawFilledCircle(x + radius, y + height - radius, radius, color);
    drawFilledCircle(x + width - radius, y + height - radius, radius, color);
}
