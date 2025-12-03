#ifndef LOGIN_MODERN_H
#define LOGIN_MODERN_H

#include "renderer.h"
#include "font_renderer.h"
#include "input_manager.h"

class ModernLogin {
public:
    ModernLogin(Renderer& renderer, FontRenderer& fontRenderer, InputManager& input);
    
    bool run();  // Returns true if login successful
    
private:
    void render();
    void renderLogo(int frame);
    void renderInputField(bool focused);
    void renderPasswordDots();
    void renderButton(bool hovered);
    void playSuccessAnimation();
    
    Renderer& m_renderer;
    FontRenderer& m_fontRenderer;
    InputManager& m_input;
    
    char m_password[64];
    int m_passwordLen;
    bool m_inputFocused;
    int m_animFrame;
    
    // UI positions
    int m_centerX, m_centerY;
    int m_inputX, m_inputY, m_inputW, m_inputH;
    int m_buttonX, m_buttonY, m_buttonW, m_buttonH;
};

#endif // LOGIN_MODERN_H
