#ifndef FONT_RENDERER_H
#define FONT_RENDERER_H

#include "renderer.h"

class FontRenderer {
public:
    FontRenderer(Renderer& renderer);
    
    void drawText(int x, int y, const char* text, Renderer::Color color, int size = 1);
    void drawTextCentered(int y, const char* text, Renderer::Color color, int size = 1);
    void present(); 
    
    int measureText(const char* text, int size = 1);

private:
    Renderer& m_renderer;
    
    // Embedded font data will be initialized at runtime
    static const uint8_t FONT_DATA[][16];
};

#endif // FONT_RENDERER_H
