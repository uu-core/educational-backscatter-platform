cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

# include(example_auto_set_url.cmake)

# Hardware-specific examples in subdirectories:
add_executable(receiver_CC2500)

# pull in common dependencies and additional spi hardware support
target_link_libraries(receiver_CC2500 pico_stdlib hardware_spi)

# stdout: enable usb output, disable uart output
pico_enable_stdio_usb(receiver_CC2500 1)
pico_enable_stdio_uart(receiver_CC2500 0)

# Add include directory 
target_sources(receiver_CC2500 PRIVATE 
        main.c
        ../project_pico_libs/packet_generation.c
        ../project_pico_libs/receiver_CC2500.c
        ../project_pico_libs/carrier_CC2500.c
)
include_directories(../project_pico_libs)

# create map/bin/hex file etc.
pico_add_extra_outputs(receiver_CC2500)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

