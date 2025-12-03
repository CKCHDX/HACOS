#ifndef DESKTOP_H
#define DESKTOP_H

#include "renderer.h"
#include "font_renderer.h"

class DesktopManager {
public:
    DesktopManager(Renderer& renderer, FontRenderer& fontRenderer);
    
    void run();
    
private:
    Renderer& m_renderer;
    FontRenderer& m_fontRenderer;
    
    void render();
    void handleInput();
};

#endif // DESKTOP_H
