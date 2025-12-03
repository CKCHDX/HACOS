#include "renderer.h"
#include "input_manager.h"
#include <cstring>

extern "C" {
    void delay_ms(int ms);
    uint64_t get_ticks();
}

// Draw large "Welcome" text
// Draw "Welcome" text - cleaner and larger
void drawWelcomeText(Renderer& r, int x, int y) {
    Renderer::Color textColor(150, 210, 255);
    int charWidth = 20;
    int charHeight = 40;
    
    // W - two vertical lines with peak
    r.drawFilledRectangle(x, y, 6, charHeight, textColor);
    r.drawFilledRectangle(x + 7, y + 15, 6, charHeight - 15, textColor);
    r.drawFilledRectangle(x + 14, y + 15, 6, charHeight - 15, textColor);
    r.drawFilledRectangle(x + 21, y, 6, charHeight, textColor);
    
    // e
    r.drawFilledRectangle(x + 40, y, 20, 6, textColor);      // top
    r.drawFilledRectangle(x + 40, y + 17, 20, 6, textColor); // middle
    r.drawFilledRectangle(x + 40, y + 34, 20, 6, textColor); // bottom
    r.drawFilledRectangle(x + 40, y, 6, charHeight, textColor); // left
    
    // l
    r.drawFilledRectangle(x + 75, y, 6, charHeight, textColor);
    
    // c
    r.drawFilledRectangle(x + 95, y, 6, charHeight, textColor);   // left line
    r.drawFilledRectangle(x + 95, y, 16, 6, textColor);           // top
    r.drawFilledRectangle(x + 95, y + charHeight - 6, 16, 6, textColor); // bottom
    
    // o
    r.drawFilledRectangle(x + 125, y, 6, charHeight, textColor);  // left
    r.drawFilledRectangle(x + 145, y, 6, charHeight, textColor);  // right
    r.drawFilledRectangle(x + 125, y, 26, 6, textColor);          // top
    r.drawFilledRectangle(x + 125, y + charHeight - 6, 26, 6, textColor); // bottom
    
    // m
    r.drawFilledRectangle(x + 165, y, 6, charHeight, textColor);  // left
    r.drawFilledRectangle(x + 177, y, 6, 20, textColor);          // middle peak
    r.drawFilledRectangle(x + 189, y, 6, charHeight, textColor);  // right
    r.drawFilledRectangle(x + 165, y + 20, 30, 6, textColor);     // baseline
    
    // e (second)
    r.drawFilledRectangle(x + 210, y, 20, 6, textColor);          // top
    r.drawFilledRectangle(x + 210, y + 17, 20, 6, textColor);     // middle
    r.drawFilledRectangle(x + 210, y + 34, 20, 6, textColor);     // bottom
    r.drawFilledRectangle(x + 210, y, 6, charHeight, textColor);  // left
}


bool runLoginScreen(uint32_t* framebuffer, uint32_t width, uint32_t height, uint32_t pitch) {
    Renderer renderer(framebuffer, width, height, pitch);
    InputManager input;
    
    char password[64] = {0};
    int passwordLen = 0;
    int cursorBlink = 0;
    bool authenticated = false;
    bool firstFrame = true;
    
    Renderer::Color bgDark(15, 15, 22);
    Renderer::Color accentColor(100, 180, 255);
    Renderer::Color accentBright(150, 210, 255);
    Renderer::Color panelBg(28, 32, 45);
    Renderer::Color inputBg(35, 40, 55);
    
    int panelX = width / 2 - 320;
    int panelY = height / 2 - 240;
    int panelW = 640;
    int panelH = 480;
    
    int inputX = panelX + 80;
    int inputY = panelY + 280;
    int inputW = panelW - 160;
    int inputH = 65;
    
    int btnX = inputX;
    int btnY = inputY + 110;
    int btnW = inputW;
    int btnH = 65;
    
    while (!authenticated) {
        // INPUT
        input.update();
        int key = input.getKey();
        
        if (key >= 0) {
            if (key == 13 || key == 10) {  // Enter
                if (passwordLen > 0) {
                    authenticated = true;
                }
            } else if (key == 8 || key == 127) {  // Backspace
                if (passwordLen > 0) {
                    password[--passwordLen] = 0;
                }
            } else if (key >= 32 && key < 127 && passwordLen < 63) {
                password[passwordLen++] = (char)key;
                password[passwordLen] = 0;
            }
        }
        
        // RENDER - only on first frame or on changes
        if (firstFrame) {
            renderer.clear(bgDark);
            firstFrame = false;
        }
        
        // Card (only draw once, not every frame)
        static bool cardDrawn = false;
        if (!cardDrawn) {
            renderer.drawFilledRectangle(panelX, panelY, panelW, panelH, panelBg);
            
            // Border
            renderer.drawFilledRectangle(panelX, panelY, panelW, 2, accentColor);
            renderer.drawFilledRectangle(panelX, panelY + panelH - 2, panelW, 2, accentColor);
            renderer.drawFilledRectangle(panelX, panelY, 2, panelH, accentColor);
            renderer.drawFilledRectangle(panelX + panelW - 2, panelY, 2, panelH, accentColor);
            
            // Welcome text area
            renderer.drawFilledRectangle(panelX + 50, panelY + 50, 540, 100, Renderer::Color(35, 42, 60));
            drawWelcomeText(renderer, panelX + 200, panelY + 65);
            
            // Decorative line
            renderer.drawFilledRectangle(panelX + 120, panelY + 175, panelW - 240, 2, accentColor);
            
            // Password input border
            renderer.drawFilledRectangle(inputX - 2, inputY - 2, inputW + 4, inputH + 4, accentColor);
            renderer.drawFilledRectangle(inputX, inputY, inputW, inputH, inputBg);
            
            // Button
            renderer.drawFilledRectangle(btnX, btnY, btnW, btnH, accentColor);
            renderer.drawFilledRectangle(btnX, btnY, btnW, 2, accentBright);
            
            // Arrow
            int arrowX = btnX + btnW / 2;
            int arrowY = btnY + btnH / 2;
            renderer.drawFilledRectangle(arrowX - 16, arrowY - 2, 32, 4, Renderer::Color(255, 255, 255));
            for (int i = 0; i < 6; i++) {
                renderer.drawFilledRectangle(arrowX + 12 - i, arrowY - 4 + i, 1, 1, Renderer::Color(255, 255, 255));
                renderer.drawFilledRectangle(arrowX + 12 - i, arrowY + 4 - i, 1, 1, Renderer::Color(255, 255, 255));
            }
            
            cardDrawn = true;
        }
        
        // Draw password dots (changes every frame)
        renderer.drawFilledRectangle(inputX + 10, inputY + 10, inputW - 20, inputH - 20, inputBg);
        
        if (passwordLen > 0) {
            int dotSize = 8;
            int dotSpacing = 24;
            int totalWidth = passwordLen * dotSpacing;
            int startX = inputX + (inputW - totalWidth) / 2;
            
            for (int i = 0; i < passwordLen; i++) {
                renderer.drawFilledCircle(startX + i * dotSpacing, inputY + inputH / 2, dotSize, accentBright);
            }
        } else {
            renderer.drawFilledRectangle(inputX + 20, inputY + inputH / 2 - 2, 100, 4, Renderer::Color(100, 120, 150));
        }
        
        // Cursor blink (changes every frame)
        cursorBlink = (cursorBlink + 1) % 60;
        if (cursorBlink < 30 && passwordLen > 0) {
            renderer.drawFilledRectangle(inputX + inputW - 30, inputY + 15, 2, inputH - 30, accentBright);
        }
        
        delay_ms(16);
    }
    
    return true;
}
