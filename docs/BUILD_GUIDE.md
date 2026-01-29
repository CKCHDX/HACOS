# HACOS Build Guide

## Quick Start

### Automated Setup (Recommended)

```bash
./setup.sh
make clean
make
make run
```

### Manual Setup

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential gnu-efi mtools qemu-system-x86 ovmf
make clean
make
make run
```

#### Arch Linux
```bash
sudo pacman -S base-devel gnu-efi mtools qemu-full edk2-ovmf
make clean
make
make run
```

## Build Process

### Step 1: Clean Previous Builds

```bash
make clean
```

This removes the `build/` directory with all compiled objects and images.

### Step 2: Compile

```bash
make
```

This command:
1. Creates `build/` directory
2. Compiles bootloader (UEFI EFI application)
3. Compiles kernel (C code)
4. Compiles userspace (C++ code)
5. Links everything into `BOOTX64.EFI`

#### Build Output

```
build/
â"œâ"€â"€ BOOTX64.EFI          # UEFI bootloader executable
â"œâ"€â"€ BOOTX64.so           # Intermediate shared object
â"œâ"€â"€ kernel.bin           # Kernel binary
â"œâ"€â"€ *.o                  # Object files
```

### Step 3: Create Disk Image

```bash
make image
```

This command:
1. Creates `build/iso/EFI/BOOT/` directory structure
2. Copies `BOOTX64.EFI` to boot directory
3. Creates 64MB FAT32 disk image
4. Copies bootloader to disk image

#### Image Output

```
build/
â"œâ"€â"€ hacos.img            # Bootable FAT32 disk image (64MB)
â""â"€â"€ iso/
    â""â"€â"€ EFI/
        â""â"€â"€ BOOT/
            â""â"€â"€ BOOTX64.EFI
```

### Step 4: Run in QEMU

```bash
make run
```

This command:
1. Runs `make image` to ensure image is up-to-date
2. Launches QEMU with:
   - OVMF UEFI firmware
   - 256MB RAM
   - Disk image mounted
   - BIOS mode

## Build Targets

### Primary Targets

| Target | Description |
|--------|-------------|
| `make` | Build all components (default) |
| `make clean` | Remove all build artifacts |
| `make image` | Create bootable disk image |
| `make run` | Build and run in QEMU |

### Component Targets

Individual components can be built separately:

```bash
make build/BOOTX64.EFI    # Build bootloader
make build/kernel.bin     # Build kernel
make build/desktop.o      # Build desktop module
```

## Compiler Flags

### Bootloader (C)

```makefile
CFLAGS = -Wall -Wextra -O2 -ffreestanding -fno-stack-protector -fpic \
         -mno-red-zone \
         -I/usr/include/efi -I/usr/include/efi/x86_64 -Iinclude \
         -DEFI_FUNCTION_WRAPPER \
         -fshort-wchar
```

### Userspace (C++)

```makefile
CXXFLAGS = -Wall -Wextra -O2 -ffreestanding -fno-exceptions -fno-rtti \
           -fno-stack-protector -mno-red-zone -Iinclude -mcmodel=large \
           -mno-mmx -mno-sse -mno-sse2 -std=c++17
```

### Kernel (C)

```makefile
KERNEL_CFLAGS = -Wall -Wextra -O2 -ffreestanding -fno-stack-protector \
                -mno-red-zone -Iinclude -mcmodel=large \
                -mno-mmx -mno-sse -mno-sse2
```

## Common Build Issues

### Issue: `efi.h: No such file or directory`

**Cause**: GNU EFI development libraries not installed

**Solution**:
```bash
# Ubuntu/Debian
sudo apt-get install gnu-efi

# Arch Linux
sudo pacman -S gnu-efi
```

### Issue: `mtools: command not found`

**Cause**: mtools package not installed

**Solution**:
```bash
# Ubuntu/Debian
sudo apt-get install mtools

# Arch Linux
sudo pacman -S mtools
```

### Issue: OVMF firmware not found

**Cause**: QEMU UEFI firmware not installed or path incorrect

**Solution**:
```bash
# Ubuntu/Debian
sudo apt-get install ovmf

# Arch Linux
sudo pacman -S edk2-ovmf

# Find OVMF location
find /usr/share -name "OVMF*.fd" 2>/dev/null
```

Update Makefile `run` target with correct path.

### Issue: Undefined reference errors

**Cause**: Missing object files or incorrect link order

**Solution**:
1. Run `make clean`
2. Check Makefile for object file dependencies
3. Rebuild with `make`

### Issue: Linker script errors

**Cause**: Incorrect path to EFI linker script

**Solution**:
Find the linker script:
```bash
find /usr/lib -name "elf_x86_64_efi.lds" 2>/dev/null
```

Update `LDFLAGS` in Makefile with correct path.

## Development Workflow

### Recommended Workflow

1. **Edit code** in `userspace/`, `kernel/`, or `boot/`
2. **Build**: `make`
3. **Test**: `make run`
4. **Iterate**: Repeat steps 1-3

### Quick Rebuild

After editing userspace code:
```bash
make build/desktop.o   # Rebuild only changed file
make                    # Relink
make run                # Test
```

### Full Clean Build

When in doubt, do a clean build:
```bash
make clean && make && make run
```

## QEMU Configuration

### Default Configuration

```bash
qemu-system-x86_64 \
    -bios /usr/share/ovmf/OVMF.fd \
    -drive file=build/hacos.img,format=raw \
    -m 256M
```

### Advanced Configuration

For debugging and better performance:

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

Options:
- `-enable-kvm`: Use hardware acceleration (Linux only)
- `-m 512`: Allocate 512MB RAM
- `-device usb-kbd`: Enable USB keyboard support
- `-serial stdio`: Redirect serial output to terminal
- `-display gtk`: Use GTK display (better graphics)

## Verification

### Successful Build Checklist

- [ ] No compilation errors
- [ ] `build/BOOTX64.EFI` created
- [ ] `build/kernel.bin` created
- [ ] `build/hacos.img` created (with `make image`)
- [ ] QEMU launches without errors
- [ ] Boot animation plays
- [ ] Login screen appears
- [ ] Desktop appears after login
- [ ] Terminal responds to keyboard

### Testing Sequence

1. **Boot Animation**: Should show orbital rings and pulsing core
2. **Login Screen**: Should display "Welcome" and password input
3. **Desktop**: Should show taskbar, terminal window
4. **Terminal**: Type `help` and verify commands work
5. **Commands**: Test each command: `about`, `version`, `sysinfo`, `echo`, `clear`
6. **Toggle**: Press ESC to hide/show terminal

## Dependencies Summary

### Required Packages

| Package | Ubuntu/Debian | Arch Linux | Purpose |
|---------|---------------|------------|---------|
| GCC/G++ | build-essential | base-devel | C/C++ compiler |
| GNU EFI | gnu-efi | gnu-efi | UEFI development |
| mtools | mtools | mtools | FAT32 tools |
| QEMU | qemu-system-x86 | qemu-full | Emulator |
| OVMF | ovmf | edk2-ovmf | UEFI firmware |

### Optional Tools

- `gdb` - Debugging
- `objdump` - Binary inspection
- `hexdump` - Binary analysis
- `git` - Version control

## Performance Notes

### Build Time

On a modern system:
- Clean build: ~5-10 seconds
- Incremental build: ~1-2 seconds
- Image creation: ~1 second

### QEMU Boot Time

- Boot animation: ~5 seconds
- Login screen: Immediate
- Desktop load: Immediate

## Next Steps

After successful build:
1. Read `docs/DESKTOP_IMPLEMENTATION.md` for desktop details
2. Check `CHANGELOG.md` for recent changes
3. Explore code in `userspace/` to understand implementation
4. Try modifying shell commands or UI elements
5. Contribute improvements!

## Support

If you encounter issues:
1. Check this guide for common problems
2. Verify all dependencies are installed
3. Try a clean build: `make clean && make`
4. Check system has enough disk space (>500MB free)
5. Ensure QEMU and OVMF are properly installed

## Resources

- **Makefile**: Build configuration
- **README.md**: Project overview
- **docs/**: Additional documentation
- **CHANGELOG.md**: Version history
