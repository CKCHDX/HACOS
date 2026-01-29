#ifndef DESKTOP_H
#define DESKTOP_H

#include "renderer.h"
#include "font_renderer.h"
#include "input_manager.h"

class DesktopManager {
public:
    DesktopManager(Renderer& renderer, FontRenderer& fontRenderer);
    
    void run();
    
private:
    Renderer& m_renderer;
    FontRenderer& m_fontRenderer;
    InputManager m_input;
    
    // Terminal state
    static const int MAX_COMMAND_LENGTH = 64;
    static const int MAX_HISTORY = 20;  // Increased from 10 for better usability
    char m_commandBuffer[MAX_COMMAND_LENGTH];
    int m_commandLength;
    char m_history[MAX_HISTORY][MAX_COMMAND_LENGTH];
    int m_historyCount;
    bool m_terminalVisible;
    int m_cursorBlink;
    
    void render();
    void handleInput();
    void executeCommand(const char* command);
    void addToHistory(const char* command);
    void drawTerminal();
    void drawTaskbar();
    void drawClock();
};

#endif // DESKTOP_H
