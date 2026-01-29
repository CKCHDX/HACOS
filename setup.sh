#!/bin/bash

# HACOS Development Environment Setup Script
# This script installs required dependencies for building HACOS

set -e

echo "================================="
echo "HACOS Development Setup"
echo "================================="
echo ""

# Detect OS
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
else
    echo "Cannot detect OS. Please install dependencies manually."
    exit 1
fi

echo "Detected OS: $OS"
echo ""

# Install dependencies based on OS
case "$OS" in
    ubuntu|debian)
        echo "Installing dependencies for Ubuntu/Debian..."
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            gnu-efi \
            mtools \
            qemu-system-x86 \
            ovmf
        ;;
    
    arch|manjaro)
        echo "Installing dependencies for Arch Linux..."
        sudo pacman -S --needed --noconfirm \
            base-devel \
            gnu-efi \
            mtools \
            qemu-full \
            edk2-ovmf
        ;;
    
    fedora|rhel|centos)
        echo "Installing dependencies for Fedora/RHEL/CentOS..."
        sudo dnf install -y \
            gcc \
            gcc-c++ \
            make \
            gnu-efi-devel \
            mtools \
            qemu-system-x86 \
            edk2-ovmf
        ;;
    
    *)
        echo "Unsupported OS: $OS"
        echo ""
        echo "Please install the following packages manually:"
        echo "  - GCC/G++ compiler"
        echo "  - GNU EFI development libraries"
        echo "  - mtools (for FAT32 image manipulation)"
        echo "  - QEMU (qemu-system-x86_64)"
        echo "  - OVMF firmware"
        exit 1
        ;;
esac

echo ""
echo "================================="
echo "Setup Complete!"
echo "================================="
echo ""
echo "You can now build HACOS:"
echo "  make clean"
echo "  make"
echo ""
echo "To run HACOS in QEMU:"
echo "  make run"
echo ""
