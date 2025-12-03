cmake_minimum_required(VERSION 3.13)

# Initialize the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico_usb_mic C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

# Add executable
add_executable(pico_usb_mic
    main.c
    usb_descriptors.c
)

# Add TinyUSB audio class support
target_compile_definitions(pico_usb_mic PRIVATE
    CFG_TUSB_CONFIG_FILE="tusb_config.h"
)

# Link libraries
target_link_libraries(pico_usb_mic
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    tinyusb_device
    tinyusb_board
)

# Enable USB output, disable UART output
pico_enable_stdio_usb(pico_usb_mic 0)
pico_enable_stdio_uart(pico_usb_mic 1)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(pico_usb_mic)

# Include directories
target_include_directories(pico_usb_mic PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)