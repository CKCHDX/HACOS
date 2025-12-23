# HACOS Keyboard Input Quick Reference

## Status: ✅ FIXED

The PS/2 keyboard input system has been fixed. This guide explains how it works.

## Current Architecture

```
User Types on Keyboard
        ↓
PS/2 Controller (0x60, 0x64)
        ↓
get_key_async() [kernel/kernel.c]
        ↓
InputManager::getKey() [userspace/input_manager.cpp]
        ↓
Login Screen [userspace/login_consumer.cpp]
```

## Key Components

### 1. PS/2 Keyboard Initialization

**File**: `kernel/kernel.c`
**Function**: `void init_ps2_keyboard()`
**Called From**: `kernel_main()` before `userspace_main()`

**What it does**:
- Initializes PS/2 controller at ports 0x60 (data) and 0x64 (status/command)
- Configures controller to enable keyboard interrupts
- Enables scancode translation (XT → AT format)
- Resets keyboard and verifies it's working
- Clears the input buffer

**Why it's needed**:
UEFI manages PS/2 hardware during boot. After `ExitBootServices()`, the controller is in an undefined state. This function properly reinitializes it.

### 2. Keyboard Data Reading

**File**: `kernel/kernel.c`
**Function**: `int get_key_async()`

**How it works**:
```
Check status port (0x64) bit 0
  └─ Has data? Read from data port (0x60)
  └─ No data? Return -1
       ↓
    Parse scancode
       ↓
    Check for Shift key (0x2A, 0x36 = pressed, 0xAA, 0xB6 = released)
       ↓
    Ignore key releases (scancode >= 0x80)
       ↓
    Convert to ASCII using lookup table
       ↓
    Return character (or -1 if invalid)
```

**Scancode Tables**:
- `scancode_to_ascii[128]` - Normal keys (a-z, 0-9, etc.)
- `scancode_to_ascii_shift[128]` - Shifted keys (A-Z, !, @, etc.)
- `shift_pressed` - Global flag tracking shift state

### 3. Input Manager

**File**: `userspace/input_manager.cpp`
**Class**: `InputManager`

**Interface**:
```cpp
InputManager input;
input.update();      // Call once per frame
int key = input.getKey();  // Get pressed key (-1 if none)
```

**What it does**:
- Wraps `get_key_async()` from kernel
- Manages input state and key tracking
- Returns one key press per frame

### 4. Login Screen

**File**: `userspace/login_consumer.cpp`
**Function**: `bool runLoginScreen()`

**Keyboard Handling**:
```cpp
input.update();
int key = input.getKey();

if (key >= 0) {
    if (key == 13 || key == 10) {      // Enter
        authenticated = true;
    } else if (key == 8 || key == 127) { // Backspace
        if (passwordLen > 0) {
            password[--passwordLen] = 0;
        }
    } else if (key >= 32 && key < 127) { // Printable character
        if (passwordLen < 63) {
            password[passwordLen++] = (char)key;
        }
    }
}
```

**Supported Keys**:
| Key | ASCII | Function |
|-----|-------|----------|
| A-Z | 65-90 | Type letter (uppercase with Shift) |
| a-z | 97-122 | Type letter |
| 0-9 | 48-57 | Type digit |
| Space | 32 | Type space |
| Backspace | 8 | Delete character |
| Enter | 13/10 | Submit password |
| Shift | varies | Modifier for uppercase |

## Data Flow Example

### User Presses 'A' (with Shift)

```
1. Keyboard sends scancode 0x2A (Shift pressed)
   get_key_async() detects shift_pressed = 1, returns -1

2. Keyboard sends scancode 0x1E ('a' key)
   get_key_async() sees shift_pressed == 1
   Uses scancode_to_ascii_shift[0x1E] = 'A'
   Returns (int)'A' = 65

3. LoginScreen receives key = 65
   password[0] = 'A'
   Renders one password dot

4. Keyboard sends scancode 0xB6 (Shift released)
   get_key_async() detects shift_released, shift_pressed = 0, returns -1
```

### User Presses Backspace

```
1. Keyboard sends scancode 0x0E (Backspace key)
   get_key_async() uses scancode_to_ascii[0x0E] = '\b' = 8
   Returns (int)'\b' = 8

2. LoginScreen receives key = 8
   Checks "if (key == 8 || key == 127)"
   Decrements passwordLen
   Removes last dot from display
```

## Port Details

### Data Port (0x60)

**Read**:
- Keyboard scancode
- Keyboard ACK/Status responses

**Write**:
- Commands to keyboard (0xFF = reset, 0xF4 = enable reporting)
- Commands to mouse (via controller)

### Status/Command Port (0x64)

**Read**:
```
Bit 0: Output buffer full (has data to read from 0x60)
Bit 1: Input buffer full (controller busy, don't send commands)
Bit 2: System flag
Bit 3: A20 gate status
Bit 4: Keyboard lock status
Bit 5: Transmit timeout
Bit 6: Receive timeout
Bit 7: Parity error
```

**Write** (Controller commands):
```
0x20: Read configuration byte
0x60: Write configuration byte
0xAD: Disable keyboard
0xAE: Enable keyboard
0xA7: Disable mouse
0xA8: Enable mouse
0xFF: Keyboard reset command
```

## Configuration Byte (Read at 0x20, Write at 0x60)

```
Bit 0: Keyboard interrupt enabled      [SET to 1]
Bit 1: Mouse interrupt enabled         [not used]
Bit 2: System flag                     [leave as-is]
Bit 3: Reserved
Bit 4: Keyboard disabled               [CLEAR to 0]
Bit 5: Mouse disabled                  [not used]
Bit 6: Scancode translation enabled    [SET to 1]
Bit 7: Reserved
```

**Our Configuration**: 0x41 (bits 0 and 6 set)
- Keyboard interrupts enabled
- Scancode translation enabled
- Keyboard port enabled

## Testing

### Manual Test
```bash
make clean
make
make run
```

**Expected behavior**:
1. HACOS boots
2. Boot animation plays
3. Login screen appears
4. Type on keyboard - characters appear as dots
5. Press Backspace - last dot disappears
6. Press Enter - login accepted

### Debug

If keyboard doesn't work:

1. Verify `init_ps2_keyboard()` is being called
   - Add debug output before/after function

2. Verify ports are accessible
   - Test reading status port
   - Should read valid status byte

3. Verify QEMU PS/2 is enabled
   - Check Makefile for QEMU arguments
   - Default should provide PS/2 keyboard

4. Check for IRQ conflicts
   - If using interrupt-driven keyboard, ensure IRQ1 is available
   - Current implementation uses polling (status bit 0)

## Future Enhancements

### Mouse Support

The infrastructure exists in `mouse_manager.cpp` but isn't used in login screen.

To add:
1. Call `init_ps2_mouse()` in kernel
2. Instantiate `MouseManager` in login screen
3. Render cursor each frame
4. Handle mouse button clicks

### Interrupt-Driven Keyboard

Current implementation polls the status port.

To use interrupts:
1. Set up IDT entry for IRQ1 (keyboard)
2. In keyboard handler: read scancode, queue key
3. In `get_key_async()`: return next queued key
4. Add debouncing logic if needed

### Extended Keys

Current implementation handles basic keys. Extended keys (arrow keys, F-keys) send two-byte sequences starting with 0xE0.

To support:
1. In `get_key_async()`: check for 0xE0 escape byte
2. Read next scancode
3. Map to extended key codes
4. Return special values (e.g., -100 for left arrow)

## Files Changed

- **kernel/kernel.c**: Added `init_ps2_keyboard()`, call in `kernel_main()`
- **docs/INPUT_SYSTEM_FIX.md**: Comprehensive technical documentation
- **docs/KEYBOARD_INPUT_GUIDE.md**: This file

## References

- PS/2 8042 Controller: [PS/2 Controller Specification](https://www.win.tue.nl/~aeb/linux/kbd/An-old-article-about-Linux-on-the-Desktop.html)
- OSDev PS/2: [OSDev Wiki - PS/2](https://wiki.osdev.org/PS/2_Keyboard)
- Keyboard Scancodes: [Scancode Reference](https://wiki.osdev.org/Keyboard_Scancodes)

## Verification Checklist

- [x] PS/2 initialization implemented
- [x] Keyboard data reading working
- [x] Scancode-to-ASCII conversion correct
- [x] Shift key handling functional
- [x] Login screen accepts input
- [x] Documentation complete
- [ ] Interrupt-driven mode (optional)
- [ ] Mouse support (optional)
- [ ] Extended key support (optional)
