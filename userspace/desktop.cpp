#include "desktop.h"

extern "C" {
    void delay_ms(int ms);
    int get_key_async();
}

// Helper function to copy strings safely
static void safe_strcpy(char* dest, const char* src, int maxLen) {
    int i;
    for (i = 0; i < maxLen - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

// Helper function to compare strings
static bool str_equals(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return false;
        a++;
        b++;
    }
    return *a == *b;
}

DesktopManager::DesktopManager(Renderer& renderer, FontRenderer& fontRenderer)
    : m_renderer(renderer), m_fontRenderer(fontRenderer), m_commandLength(0),
      m_historyCount(0), m_terminalVisible(true), m_cursorBlink(0) {
    // Initialize command buffer
    for (int i = 0; i < MAX_COMMAND_LENGTH; i++) {
        m_commandBuffer[i] = '\0';
    }
    // Initialize history
    for (int i = 0; i < MAX_HISTORY; i++) {
        for (int j = 0; j < MAX_COMMAND_LENGTH; j++) {
            m_history[i][j] = '\0';
        }
    }
}

void DesktopManager::drawClock() {
    // Simple static clock for now
    m_fontRenderer.drawText(m_renderer.width() - 100, m_renderer.height() - 32, 
                           "14:23", Renderer::Color(200, 200, 200), 2);
}

void DesktopManager::drawTaskbar() {
    int taskbarHeight = 50;
    int taskbarY = m_renderer.height() - taskbarHeight;
    
    // Taskbar background with slight transparency
    m_renderer.drawFilledRectangle(0, taskbarY, 
                                   m_renderer.width(), taskbarHeight, 
                                   Renderer::Color(10, 15, 25, 230));
    
    // HACOS logo in taskbar
    m_renderer.drawFilledCircle(40, taskbarY + 25, 15, 
                               Renderer::Color(6, 182, 212));
    
    // System name
    m_fontRenderer.drawText(70, taskbarY + 17, 
                           "HACOS", Renderer::Color(6, 182, 212), 2);
    
    // Draw clock
    drawClock();
}

void DesktopManager::drawTerminal() {
    if (!m_terminalVisible) return;
    
    int termX = 100;
    int termY = 100;
    int termW = m_renderer.width() - 200;
    int termH = 400;
    
    // Terminal window with shadow
    m_renderer.drawFilledRectangle(termX + 3, termY + 3, termW, termH, 
                                  Renderer::Color(0, 0, 0, 100));
    
    // Terminal background
    m_renderer.drawFilledRectangle(termX, termY, termW, termH, 
                                  Renderer::Color(20, 25, 35, 240));
    
    // Terminal border
    m_renderer.drawRectangle(termX, termY, termW, termH, 
                            Renderer::Color(6, 182, 212));
    
    // Title bar
    m_renderer.drawFilledRectangle(termX, termY, termW, 30, 
                                  Renderer::Color(6, 182, 212));
    m_fontRenderer.drawText(termX + 10, termY + 8, 
                           "HACOS Terminal", Renderer::Color(255, 255, 255), 1);
    
    // Draw command history
    int lineY = termY + 50;
    int lineHeight = 25;
    
    for (int i = 0; i < m_historyCount && i < MAX_HISTORY; i++) {
        m_fontRenderer.drawText(termX + 15, lineY, "> ", 
                               Renderer::Color(6, 182, 212), 1);
        m_fontRenderer.drawText(termX + 35, lineY, m_history[i], 
                               Renderer::Color(200, 200, 200), 1);
        lineY += lineHeight;
    }
    
    // Draw current command line
    m_fontRenderer.drawText(termX + 15, lineY, "> ", 
                           Renderer::Color(6, 182, 212), 1);
    
    // Draw command buffer
    if (m_commandLength > 0) {
        m_fontRenderer.drawText(termX + 35, lineY, m_commandBuffer, 
                               Renderer::Color(200, 200, 200), 1);
    }
    
    // Blinking cursor
    m_cursorBlink = (m_cursorBlink + 1) % 60;
    if (m_cursorBlink < 30) {
        int cursorX = termX + 35 + (m_commandLength * 8);
        m_renderer.drawFilledRectangle(cursorX, lineY, 8, 16, 
                                      Renderer::Color(6, 182, 212));
    }
    
    // Help text at bottom
    m_fontRenderer.drawText(termX + 15, termY + termH - 40, 
                           "Commands: help, about, clear, version", 
                           Renderer::Color(100, 120, 150), 1);
}

void DesktopManager::render() {
    // Modern desktop background with gradient effect
    Renderer::Color bgTop(15, 20, 35);
    Renderer::Color bgBottom(20, 30, 50);
    
    // Simple two-tone background
    m_renderer.clear(bgTop);
    for (int y = m_renderer.height() / 2; y < m_renderer.height(); y++) {
        m_renderer.drawFilledRectangle(0, y, m_renderer.width(), 1, bgBottom);
    }
    
    // Draw terminal window
    drawTerminal();
    
    // Draw taskbar on top
    drawTaskbar();
}

void DesktopManager::addToHistory(const char* command) {
    if (m_historyCount < MAX_HISTORY) {
        safe_strcpy(m_history[m_historyCount], command, MAX_COMMAND_LENGTH);
        m_historyCount++;
    } else {
        // Shift history up
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            safe_strcpy(m_history[i], m_history[i + 1], MAX_COMMAND_LENGTH);
        }
        safe_strcpy(m_history[MAX_HISTORY - 1], command, MAX_COMMAND_LENGTH);
    }
}

void DesktopManager::executeCommand(const char* command) {
    // Add command to history
    addToHistory(command);
    
    // Simple command execution
    if (str_equals(command, "help")) {
        addToHistory("  Available commands:");
        addToHistory("  help, about, clear, version");
    } else if (str_equals(command, "about")) {
        addToHistory("  HACOS - High-Performance Amateur");
        addToHistory("  Computing Operating System");
    } else if (str_equals(command, "version")) {
        addToHistory("  HACOS v0.1.0-alpha");
    } else if (str_equals(command, "clear")) {
        m_historyCount = 0;
    } else if (command[0] != '\0') {
        addToHistory("  Unknown command. Type 'help'");
    }
}

void DesktopManager::run() {
    while (true) {
        render();
        handleInput();
        delay_ms(16);  // ~60 FPS
    }
}

void DesktopManager::handleInput() {
    m_input.update();
    int key = m_input.getKey();
    
    if (key == -1) {
        return;  // No key pressed
    }
    
    // Handle backspace
    if (key == '\b') {
        if (m_commandLength > 0) {
            m_commandLength--;
            m_commandBuffer[m_commandLength] = '\0';
        }
        return;
    }
    
    // Handle enter
    if (key == '\n') {
        if (m_commandLength > 0) {
            executeCommand(m_commandBuffer);
            // Clear command buffer
            m_commandLength = 0;
            for (int i = 0; i < MAX_COMMAND_LENGTH; i++) {
                m_commandBuffer[i] = '\0';
            }
        }
        return;
    }
    
    // Handle ESC to toggle terminal
    if (key == 27) {
        m_terminalVisible = !m_terminalVisible;
        return;
    }
    
    // Add character to buffer
    if (m_commandLength < MAX_COMMAND_LENGTH - 1 && key >= 32 && key < 127) {
        m_commandBuffer[m_commandLength] = (char)key;
        m_commandLength++;
        m_commandBuffer[m_commandLength] = '\0';
    }
}
