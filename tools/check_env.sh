#!/bin/bash
# HACOS Environment Check Script
set -e

REQUIRED_PKGS=(x86_64-elf-gcc qemu-system-x86_64 ovmf xorriso make)
MISSING=()

for pkg in "${REQUIRED_PKGS[@]}"; do
    if ! command -v $pkg &>/dev/null; then
        MISSING+=("$pkg")
    fi
    done

if [ ${#MISSING[@]} -ne 0 ]; then
    echo "Missing required tools: ${MISSING[@]}"
    echo "Install them with: sudo apt install ${MISSING[@]}"
    exit 1
fi

# Check for EDK II headers (efi.h)
if ! find /usr/include -name efi.h | grep -q efi.h; then
    echo "UEFI headers (efi.h) not found. Install EDK II or place efi.h in src/include."
    exit 1
fi

# Test build
make clean && make
if [ ! -f hacos.iso ]; then
    echo "ISO creation failed. Check Makefile and xorriso installation."
    exit 1
fi

echo "Environment is ready. You can build and run HACOS!"
