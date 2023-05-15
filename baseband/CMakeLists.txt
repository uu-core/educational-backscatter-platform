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
add_executable(pio_backscatter)

# by default the header is generated into the build dir
pico_generate_pio_header(pio_backscatter ${CMAKE_CURRENT_LIST_DIR}/backscatter.pio)
# however, alternatively you can choose to generate it somewhere else (in this case in the source tree for check in)
#pico_generate_pio_header(pio_backscatter ${CMAKE_CURRENT_LIST_DIR}/backscatter.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR})

target_sources(pio_backscatter PRIVATE 
    main.c 
    ../project_pico_libs/packet_generation.c
)
include_directories(../project_pico_libs)
target_link_libraries(pio_backscatter PRIVATE pico_stdlib hardware_pio)

pico_add_extra_outputs(pio_backscatter)

# stdout: enable usb output, disable uart output
pico_enable_stdio_usb(pio_backscatter 1)
pico_enable_stdio_uart(pio_backscatter 0)   

# add url via pico_set_program_url
# example_auto_set_url(pio_backscatter)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

