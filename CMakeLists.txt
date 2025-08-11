cmake_minimum_required(VERSION 3.13)

# Include the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(sd_card_reader)

# Initialize the SDK
pico_sdk_init()

# Add executable
add_executable(sd_card_reader
    src/main.c
    src/sd_card.c
    src/fatfs_disk.c
)

# Pull in common dependencies
target_link_libraries(sd_card_reader 
    pico_stdlib 
    hardware_spi
    hardware_gpio
    pico_multicore
)

# Enable USB output, disable UART output
pico_enable_stdio_usb(sd_card_reader 1)
pico_enable_stdio_uart(sd_card_reader 0)

# Create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(sd_card_reader)

# Add the standard library to the build
target_link_libraries(sd_card_reader pico_stdlib)
