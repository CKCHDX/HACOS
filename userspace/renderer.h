#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>

class Renderer {
public:
    struct Color {
        uint8_t r, g, b, a;
        
        constexpr Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
            : r(red), g(green), b(blue), a(alpha) {}
        
        uint32_t toRGBA() const {
            return (r << 16) | (g << 8) | b;
        }
    };
    
    Renderer(uint32_t* fb, uint32_t width, uint32_t height, uint32_t pitch);
    
    void clear(Color color);
    void drawPixel(int x, int y, Color color);
    void drawCircle(int cx, int cy, int radius, Color color);
    void drawFilledCircle(int cx, int cy, int radius, Color color);
    void drawRectangle(int x, int y, int width, int height, Color color);
    void drawFilledRectangle(int x, int y, int width, int height, Color color);
    void drawRoundedRect(int x, int y, int width, int height, int radius, Color color);
    
    void setAlpha(uint8_t alpha) { m_globalAlpha = alpha; }
    
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }

private:
    uint32_t* m_framebuffer;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_pitch;
    uint8_t m_globalAlpha;
    
    uint32_t blend(uint32_t fg, uint32_t bg, uint8_t alpha);
    int distanceSquared(int x1, int y1, int x2, int y2);
};

#endif // RENDERER_H
