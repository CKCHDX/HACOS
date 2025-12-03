#include "desktop.h"

extern "C" {
    void delay_ms(int ms);
    int get_key_async();
}

DesktopManager::DesktopManager(Renderer& renderer, FontRenderer& fontRenderer)
    : m_renderer(renderer), m_fontRenderer(fontRenderer) {}

void DesktopManager::render() {
    // Modern desktop background
    m_renderer.clear(Renderer::Color(15, 20, 35));
    
    // Taskbar
    m_renderer.drawFilledRectangle(0, m_renderer.height() - 50, 
                                   m_renderer.width(), 50, 
                                   Renderer::Color(10, 15, 25, 230));
    
    // HACOS logo in taskbar
    m_renderer.drawFilledCircle(40, m_renderer.height() - 25, 15, 
                               Renderer::Color(6, 182, 212));
    
    // Clock placeholder
    m_fontRenderer.drawText(m_renderer.width() - 100, m_renderer.height() - 32, 
                           "14:23", Renderer::Color(200, 200, 200), 2);
    
    // Welcome message
    m_fontRenderer.drawTextCentered(100, "WELCOME TO HACOS", 
                                   Renderer::Color(6, 182, 212), 3);
    
    m_fontRenderer.drawTextCentered(160, "DESKTOP ENVIRONMENT READY", 
                                   Renderer::Color(150, 150, 150), 1);
}

void DesktopManager::run() {
    render();
    
    while (true) {
        handleInput();
        delay_ms(16);
    }
}

void DesktopManager::handleInput() {
    int key = get_key_async();
    (void)key;  // Handle desktop input later
}
