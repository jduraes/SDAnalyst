#!/bin/bash

# Flash SD card reader firmware to Raspberry Pi Pico

UF2_FILE="build/sd_card_reader.uf2"

if [ ! -f "$UF2_FILE" ]; then
    echo "Error: UF2 file not found. Please build the project first."
    echo "Run: cd ~/pico/sd_card_reader && export PICO_SDK_PATH=~/pico/pico-sdk && mkdir -p build && cd build && cmake .. && make -j4"
    exit 1
fi

echo "Looking for Pico in bootloader mode..."

# Check for RPI-RP2 mount point
MOUNT_POINT="/media/$(whoami)/RPI-RP2"
if [ -d "$MOUNT_POINT" ]; then
    echo "Found Pico mounted at $MOUNT_POINT"
    echo "Copying firmware..."
    cp "$UF2_FILE" "$MOUNT_POINT/"
    echo "Firmware copied! Pico should reboot automatically."
    exit 0
fi

# Try using picotool
if command -v picotool >/dev/null 2>&1; then
    echo "Trying picotool..."
    if picotool info >/dev/null 2>&1; then
        echo "Found Pico, loading firmware..."
        picotool load "$UF2_FILE" -f
        echo "Rebooting Pico..."
        picotool reboot
        echo "Done!"
        exit 0
    fi
fi

echo "Error: Could not find Pico in bootloader mode."
echo "Please:"
echo "1. Disconnect the Pico from USB"
echo "2. Hold the BOOTSEL button"
echo "3. Connect the Pico to USB while holding BOOTSEL"
echo "4. Release BOOTSEL"
echo "5. Run this script again"
