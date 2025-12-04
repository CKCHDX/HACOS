# HACOS - High-Performance Amateur Computing Operating System

A custom UEFI-based x86_64 operating system written in C and C++, featuring a modern login interface, custom graphics rendering, and keyboard input handling. Built from scratch with a focus on low-level systems programming and elegant UI design.

## ðŸš€ Features

- **UEFI Bootloader** - EFI firmware support with graphical boot animation
- **Custom Graphics Engine** - Direct framebuffer manipulation with anti-aliased rendering
- **Modern Login Screen** - Minimalist dark-themed authentication interface with visual feedback
- **Keyboard Input System** - PS/2 and USB keyboard support via low-level port I/O
- **C++ Userspace** - Type-safe application layer with custom renderer and input manager
- **Boot Animations** - Orbital rings, pulsing cores, and smooth transitions
- **Professional UI** - Glass panel effects, gradient backgrounds, and cursor blinking

## ðŸ“‹ System Requirements

### Build Requirements
- Linux (Arch, Ubuntu, Debian, etc.)
- `gcc` / `g++` (with x86_64 EFI support)
- `gnu-efi` development libraries
- `mtools` (for FAT32 image manipulation)
- `make`
- OVMF firmware (for QEMU testing)

### Runtime Requirements (QEMU)
- QEMU 6.0+
- KVM enabled (for `-enable-kvm` option)
- 512MB+ RAM allocated

### Installation

**Ubuntu/Debian:**
```bash
sudo apt-get install build-essential gnu-efi mtools qemu-system-x86 ovmf
```

**Arch Linux:**
```bash
sudo pacman -S base-devel gnu-efi mtools qemu-full edk2-ovmf
```

## ðŸ—ï¸ Project Structure

```
HACOS/
â”œâ”€â”€ boot/                    # UEFI bootloader
â”‚   â”œâ”€â”€ uefi_main.c         # UEFI entry point & firmware initialization
â”‚   â””â”€â”€ boot_ui.c           # Boot animation & UI
â”œâ”€â”€ kernel/                  # Low-level kernel
â”‚   â”œâ”€â”€ kernel.c            # Kernel main, PS/2 keyboard driver
â”‚   â””â”€â”€ kernel.ld           # Linker script
â”œâ”€â”€ drivers/                 # Hardware drivers
â”‚   â”œâ”€â”€ graphics.c          # Graphics primitives
â”‚   â””â”€â”€ graphics.h          # Graphics header
â”œâ”€â”€ ui/                      # UI framework
â”‚   â””â”€â”€ boot_ui.h           # Boot UI definitions
â”œâ”€â”€ userspace/              # User-space applications
â”‚   â”œâ”€â”€ main.cpp            # Entry point for userspace
â”‚   â”œâ”€â”€ renderer.cpp        # Graphics rendering engine
â”‚   â”œâ”€â”€ renderer.h          # Renderer API
â”‚   â”œâ”€â”€ input_manager.cpp   # Keyboard input handling
â”‚   â”œâ”€â”€ input_manager.h     # Input manager API
â”‚   â”œâ”€â”€ login_consumer.cpp  # Login screen implementation
â”‚   â”œâ”€â”€ font_renderer.cpp   # Text rendering
â”‚   â”œâ”€â”€ gfx_effects.cpp     # Visual effects
â”‚   â”œâ”€â”€ mouse_manager.cpp   # Mouse input (WIP)
â”‚   â”œâ”€â”€ memory.cpp          # Memory management
â”‚   â”œâ”€â”€ desktop.cpp         # Desktop environment
â”‚   â””â”€â”€ include/            # Headers
â”œâ”€â”€ Makefile                # Build configuration
â””â”€â”€ README.md              # This file
```

## ðŸ”¨ Building

### Full Build
```bash
make clean
make
```

### Build Output
- `build/BOOTX64.EFI` - UEFI bootloader executable
- `build/kernel.bin` - Kernel binary
- `build/hacos.img` - FAT32 disk image (64MB)

### Creating Disk Image
```bash
make image
```

## â–¶ï¸ Running

### QEMU (Recommended)
```bash
make run
```

This command:
1. Compiles the entire project
2. Creates a bootable FAT32 disk image
3. Copies OVMF firmware variables
4. Launches QEMU with USB keyboard support

### QEMU with Debugging
```bash
qemu-system-x86_64 \
    -enable-kvm \
    -m 512 \
    -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
    -drive if=pflash,format=raw,file=build/OVMF_VARS_4M.fd \
    -drive format=raw,file=build/hacos.img \
    -device usb-ehci \
    -device usb-kbd \
    -display gtk \
    -serial stdio
```

## ðŸŽ® Usage

### Boot Process
1. **UEFI Firmware** loads BOOTX64.EFI
2. **Boot Animation** displays for ~5 seconds with orbital rings and pulsing core
3. **Kernel Initialization** sets up PS/2 keyboard driver
4. **Userspace Launch** initializes graphics renderer
5. **Login Screen** appears for user authentication

### Login Screen Controls
- **Type** - Enter password (displayed as dots for security)
- **Backspace** - Delete last character
- **ESC** - Clear entire password
- **Enter** - Submit password and authenticate

## ðŸ—ï¸ Architecture

### Boot Flow
```
UEFI Firmware
    â†“
BOOTX64.EFI (uefi_main.c)
    â†“
Boot UI Animation (boot_ui.c)
    â†“
Exit Boot Services
    â†“
kernel_main() (kernel.c)
    â†“
userspace_main() (main.cpp)
    â†“
runLoginScreen() (login_consumer.cpp)
    â†“
Desktop/Shell
```

### Input Pipeline
```
PS/2 Keyboard Port 0x60/0x64
    â†“
get_key_async() [kernel.c]
    â†“
InputManager::update() [input_manager.cpp]
    â†“
InputManager::getKey() [input_manager.cpp]
    â†“
runLoginScreen() [login_consumer.cpp]
    â†“
Password Buffer & UI Update
```

### Graphics Pipeline
```
Framebuffer (GOP Mode)
    â†“
Renderer Class [renderer.cpp]
    â†“
Primitives: Pixels, Circles, Rectangles, Text
    â†“
UI Components: Panels, Buttons, Input Fields
    â†“
Display Output
```

## ðŸ“š Key Components

### Renderer (renderer.cpp)
Low-level graphics primitive library with support for:
- Pixel manipulation
- Anti-aliased circles
- Filled rectangles
- Color blending with alpha transparency
- Distance-based rendering

**API:**
```cpp
Renderer(uint32_t* framebuffer, uint32_t width, uint32_t height, uint32_t pitch);
void clear(Color color);
void drawPixel(int x, int y, Color color);
void drawCircle(int cx, int cy, int radius, Color color);
void drawFilledCircle(int cx, int cy, int radius, Color color);
void drawRectangle(int x, int y, int width, int height, Color color);
void drawFilledRectangle(int x, int y, int width, int height, Color color);
void drawRoundedRect(int x, int y, int w, int h, int radius, Color color);
```

### InputManager (input_manager.cpp)
Hardware input abstraction layer:
- Reads from PS/2 keyboard port asynchronously
- Tracks key state to prevent repeat
- Converts scancodes to ASCII
- Supports Shift modifier key
- Mouse support (planned)

**API:**
```cpp
InputManager();
void update();
int getKey();  // Returns -1 if no new key, or key code
bool isKeyPressed(int key);
bool isMouseInRect(int x, int y, int width, int height);
```

### Login Screen (login_consumer.cpp)
Modern authentication interface featuring:
- Centered card-style panel with blue accent borders
- "Welcome" text rendered as ASCII art
- Password input with secure dot display
- Blinking cursor feedback
- Submit button with arrow icon
- Success animation on authentication

## ðŸ”Œ Hardware Support

### Keyboard
- **PS/2 Keyboard Driver** - Port-based I/O via 0x60/0x64
- **Scancode Translation** - Support for US keyboard layout
- **Shift Modifier** - Uppercase letter input
- **Key Release Detection** - Prevents key repeat
- **USB Keyboard** - QEMU USB pass-through

### Graphics
- **UEFI GOP** - Graphics Output Protocol
- **Direct Framebuffer Access** - Linear frame buffer
- **Color Depths** - 32-bit RGBA (8:8:8:8)
- **Resolution Support** - Any resolution via GOP

### Devices Supported in QEMU
- Intel i440FX chipset
- PIIX3 southbridge
- USB EHCI (for keyboard)
- PS/2 keyboard (fallback)

## ðŸŽ¨ Design Philosophy

**HACOS** is built with several design principles:

1. **Minimalism** - Clean, focused interfaces without clutter
2. **Performance** - Direct hardware access without abstraction overhead
3. **Clarity** - Self-documenting code with clear architecture
4. **Modularity** - Separate concerns (boot, kernel, userspace)
5. **Visual Polish** - Professional appearance rivaling commercial OS

## ðŸ”§ Customization

### Boot Animation Duration
Edit `boot_ui.c`, function `show_boot_animation()`:
```c
show_boot_animation(gop, 5000);  // 5000ms = 5 seconds
```

### Login Screen Colors
Edit `login_consumer.cpp`, color definitions:
```cpp
Renderer::Color bgDark(15, 15, 22);           // Dark blue background
Renderer::Color accentColor(100, 180, 255);   // Light blue accents
Renderer::Color accentBright(150, 210, 255);  // Brighter blue
```

### Screen Resolution
Automatically detected from UEFI GOP - supports any resolution

## ðŸ› Troubleshooting

### Black Screen After Boot
**Issue:** Login screen doesn't appear after boot animation
**Solutions:**
1. Check QEMU OVMF paths are correct
2. Ensure USB keyboard is enabled: `-device usb-ehci -device usb-kbd`
3. Verify framebuffer is initialized correctly

### Keyboard Not Working
**Issue:** Cannot type in login screen
**Solutions:**
1. Ensure `-device usb-kbd` is passed to QEMU
2. Check PS/2 keyboard driver is enabled in kernel.c
3. Verify `InputManager` is being updated each frame
4. Try with `-serial stdio` to see kernel debug output

### Compilation Errors
**Issue:** Build fails with undefined references
**Solutions:**
1. Install `gnu-efi-devel` package
2. Update Makefile paths to match your system
3. Run `make clean` before rebuilding

### OVMF Firmware Not Found
**Issue:** QEMU cannot find OVMF files
**Solutions:**
```bash
# Find OVMF on your system
sudo find /usr/share -name "OVMF_CODE*.fd"
sudo find /usr/share -name "OVMF_VARS*.fd"

# Update paths in Makefile's run target
```

## ðŸ“– Technical Details

### Memory Layout
```
0x0000 - 0xFFFFF         | Real mode / BIOS area
0x100000 (1MB)           | Kernel load address
0x100000+                | Kernel code & data
...                      | Stack & heap
FFFFF000 - FFFFFFFF      | UEFI reserved
```

### Port I/O
- **0x60** - PS/2 Keyboard Data Port
- **0x64** - PS/2 Keyboard Status Port

### Scancode Processing
Raw PS/2 scancodes â†’ ASCII conversion with lookup tables

## ðŸ“ License

This project is custom OS development for educational purposes. Feel free to use, modify, and learn from the codebase.

## ðŸ‘¨â€ðŸ’» Development

### Active Projects
- Login screen keyboard interaction
- Desktop environment
- File system support
- Window manager
- System calls & processes

### Future Features
- [ ] File system (ext2/FAT32)
- [ ] Process management
- [ ] Multi-tasking kernel
- [ ] GUI window manager
- [ ] Networking stack
- [ ] Command-line shell

## ðŸ”— Related Projects

- **Dynamic-OS** - Advanced OS features
- **Klar-Engine** - Graphics engine
- **PROJECT-AION** - System architecture
- **CDOX** - Documentation system

## ðŸ“ž Support

For issues, debugging, or feature requests:
1. Check kernel debug output: `make run` with `-serial stdio`
2. Verify hardware compatibility
3. Review boot log and error messages
4. Test with latest QEMU version

---


**HACOS** - Built with C, C++, and passion for low-level systems programming.
