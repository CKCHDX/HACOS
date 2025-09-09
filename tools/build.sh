#!/bin/bash

# HACOS build script
set -e

cd "$(dirname "$0")/.."

make clean
make
echo "Build complete. ISO is hacos.iso."