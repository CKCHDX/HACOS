# UEFI Hello OS Project

This project is a simple UEFI application that outputs "Hello" to the screen. It serves as a basic example of UEFI application development using the EDK II framework.

## Project Structure

```

# HACOS UEFI OS Project

This project is a modular UEFI operating system base. It boots, initializes graphics, and displays a logo window. You can easily extend it to add kernel features, GUI, drivers, and security.

## Project Structure

```
HACOS/
├── src/
│   ├── main.c        # UEFI entry point
│   ├── kernel.c      # Kernel logic
│   ├── graphics.c    # Graphics abstraction
│   ├── task.c        # Task manager
│   ├── graphics.h    # Graphics header
│   ├── task.h        # Task manager header
│   └── include/
│       └── uefi.h    # UEFI definitions
├── Makefile          # Build automation
├── README.md         # Project documentation
├── tools/
│   └── build.sh      # Build script
```

## Prerequisites (Linux)

Install these packages on Ubuntu/Debian:

```bash
sudo apt update
sudo apt install -y build-essential xorriso qemu ovmf gcc-10 gcc-10-x86-64-linux-gnu
```

You may need to build or install `x86_64-elf-gcc` cross-compiler. On Ubuntu:

```bash
sudo apt install gcc-x86-64-elf
```

## Building and Running

1. Clean previous builds:
    ```bash
    make clean
    ```
2. Build the project:
    ```bash
    make
    ```
3. Run in QEMU:
    ```bash
    make run
    ```

This will build a bootable ISO (`hacos.iso`) and launch QEMU with UEFI firmware (OVMF).

## Real Hardware

Write `hacos.iso` to a USB stick and boot on a UEFI-capable machine.

## License

MIT License. See LICENSE for details.