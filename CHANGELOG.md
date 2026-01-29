# HACOS Changelog

All notable changes to the HACOS project will be documented in this file.

## [0.2.0] - 2026-01-29

### Added
- **Interactive Desktop Environment**: Desktop now launches after successful login
- **Terminal/Shell Window**: Fully functional command-line interface with:
  - Command input with blinking cursor
  - Command history (up to 20 commands)
  - Built-in commands: `help`, `about`, `clear`, `version`, `sysinfo`, `echo`
  - ESC key to toggle terminal visibility
  - Visual feedback with cyan accent colors
- **Enhanced Taskbar**: Improved taskbar with HACOS logo and system name
- **Setup Script**: Added `setup.sh` for automated dependency installation
- **Gradient Background**: Desktop background with two-tone gradient effect
- **System Information**: `sysinfo` command displays CPU, mode, and kernel info

### Changed
- Main entry point now transitions from login to desktop environment
- Desktop manager now uses InputManager for better keyboard handling
- Improved desktop rendering with proper layering
- Version updated to 0.2.0-alpha

### Technical Details
- Desktop window with shadow effects
- Terminal positioned at center of screen
- Real-time command processing
- No external dependencies beyond existing codebase

## [0.1.0] - Previous

### Features
- UEFI bootloader with boot animation
- PS/2 keyboard driver with proper initialization
- Login screen with password input
- Graphics rendering engine
- Font rendering system
- Basic desktop stub
