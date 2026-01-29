# HACOS Desktop Implementation Guide

## Overview

This document describes the desktop environment and terminal shell implementation added to HACOS.

## Architecture

### Component Flow

```
Login Screen (login_consumer.cpp)
    â†"
Main Entry Point (main.cpp)
    â†"
Desktop Manager (desktop.cpp)
    â†"
Terminal Shell Interface
```

## Desktop Manager

### Class: `DesktopManager`

Located in: `userspace/desktop.cpp` and `userspace/desktop.h`

#### Responsibilities
- Manages the desktop environment after login
- Renders the taskbar and background
- Handles the interactive terminal window
- Processes keyboard input for command execution
- Maintains command history

#### Key Features

1. **Taskbar**
   - Bottom-aligned with 50px height
   - HACOS logo (cyan circle)
   - System name display
   - Clock placeholder (static for now)

2. **Terminal Window**
   - Centered on screen
   - Shadow effect for depth
   - Title bar with "HACOS Terminal"
   - Command prompt with "> " prefix
   - Blinking cursor for visual feedback
   - Command history display (last 10 commands)
   - Help text at bottom

3. **Command System**
   - Real-time command input
   - Immediate execution on Enter
   - Command history management
   - Built-in command handlers

## Terminal Commands

### Implemented Commands

| Command | Description | Output |
|---------|-------------|--------|
| `help` | Display available commands | Lists all commands with descriptions |
| `about` | About HACOS | Project description |
| `version` | Version information | Shows v0.2.0-alpha and build date |
| `sysinfo` | System information | CPU, mode, kernel type |
| `echo` | Echo test | Confirms terminal is working |
| `clear` | Clear history | Removes all command history |

### Adding New Commands

To add a new command to the terminal:

1. Open `userspace/desktop.cpp`
2. Locate the `executeCommand` function
3. Add a new `else if` block:

```cpp
else if (str_equals(command, "mycommand")) {
    addToHistory("  Output line 1");
    addToHistory("  Output line 2");
}
```

4. Update the help command output to include your new command

## Input Handling

### Keyboard Controls

- **Alphanumeric keys**: Add characters to command buffer
- **Enter**: Execute current command
- **Backspace**: Delete last character
- **ESC**: Toggle terminal visibility

### Input Manager Integration

The desktop uses `InputManager` class for keyboard input:
- Polls keyboard state each frame
- Converts scancodes to ASCII
- Handles special keys (Enter, Backspace, ESC)
- Prevents key repeat issues

## Rendering

### Frame Loop

The desktop runs at approximately 60 FPS (16ms delay):

```cpp
while (true) {
    render();        // Draw everything
    handleInput();   // Process keyboard
    delay_ms(16);    // 16ms delay (~60 FPS)
}
```

### Render Order

1. Background gradient (two-tone)
2. Terminal window (if visible)
3. Taskbar (always on top)

### Visual Elements

- **Background**: Gradient from dark blue-gray to lighter blue-gray
- **Terminal**: Semi-transparent dark background with cyan border
- **Text**: White and cyan colors for contrast
- **Cursor**: Blinking cyan rectangle (30 frames on, 30 frames off)

## Memory Management

### Static Buffers

The desktop uses fixed-size arrays to avoid dynamic allocation:

```cpp
char m_commandBuffer[MAX_COMMAND_LENGTH];     // Current command
char m_history[MAX_HISTORY][MAX_COMMAND_LENGTH];  // Command history
```

- `MAX_COMMAND_LENGTH`: 64 characters
- `MAX_HISTORY`: 10 commands

### String Operations

Custom string functions are used instead of standard library:
- `safe_strcpy`: Safe string copy with length limit
- `str_equals`: String comparison

## Future Enhancements

### Planned Features

1. **Window Management**
   - Multiple terminal windows
   - Window dragging
   - Minimize/maximize/close buttons

2. **Advanced Commands**
   - `ls` - List files (requires file system)
   - `cd` - Change directory
   - `cat` - Display file contents
   - `pwd` - Print working directory

3. **File System Support**
   - Integration with ext2/FAT32
   - File browser
   - File operations

4. **Process Management**
   - Background processes
   - Process list
   - Kill command

5. **System Services**
   - Real-time clock display
   - System monitor (CPU, memory usage)
   - Network status (future)

## Testing

### Manual Testing Steps

1. Build HACOS:
   ```bash
   make clean && make
   ```

2. Run in QEMU:
   ```bash
   make run
   ```

3. Test sequence:
   - Wait for boot animation
   - Login screen appears
   - Type any password and press Enter
   - Desktop appears with terminal
   - Type `help` and press Enter
   - Try other commands: `about`, `version`, `sysinfo`
   - Press ESC to hide/show terminal
   - Press ESC again to show terminal
   - Type `clear` to clear history

### Expected Behavior

- Terminal should respond immediately to keyboard input
- Cursor should blink smoothly
- Commands should execute and show output
- No crashes or hangs
- ESC toggle should work smoothly

## Troubleshooting

### Terminal not responding
- Check that InputManager is being updated each frame
- Verify PS/2 keyboard is initialized in kernel
- Ensure USB keyboard is enabled in QEMU

### Text not rendering
- Check FontRenderer is initialized properly
- Verify framebuffer is valid
- Check text coordinates are within screen bounds

### Commands not executing
- Verify command string comparison is working
- Check command buffer is being cleared after execution
- Ensure history array has space

## Code Style

### Naming Conventions

- Classes: `PascalCase` (e.g., `DesktopManager`)
- Methods: `camelCase` (e.g., `executeCommand`)
- Members: `m_` prefix (e.g., `m_renderer`)
- Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_COMMAND_LENGTH`)

### Formatting

- Indent: 4 spaces
- Braces: Same line for functions, next line for classes
- Comments: C++ style `//` for single line

## Resources

- **Main Files**:
  - `userspace/desktop.cpp` - Desktop implementation
  - `userspace/desktop.h` - Desktop header
  - `userspace/main.cpp` - Entry point

- **Dependencies**:
  - `renderer.cpp/h` - Graphics rendering
  - `font_renderer.cpp/h` - Text rendering
  - `input_manager.cpp/h` - Keyboard input

- **Documentation**:
  - `README.md` - Project overview
  - `CHANGELOG.md` - Version history
  - This file - Implementation guide
