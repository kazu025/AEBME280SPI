cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(bme280 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(bme280
        main.cpp
        aebme280.cpp
        )

# pull in common dependencies and additional spi hardware support
target_link_libraries(bme280
        pico_stdlib
        hardware_spi
        pico_cyw43_arch_none)

pico_enable_stdio_usb(bme280 1)
pico_enable_stdio_uart(bme280 1)
# create map/bin/hex file etc.
pico_add_extra_outputs(bme280)

