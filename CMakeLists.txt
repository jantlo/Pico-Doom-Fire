cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(pico-doom-fire)

pico_sdk_init()

#add_subdirectory(src)

file(GLOB SOURCE_FILE
src/*.h
src/*.cpp
)

add_executable(main ${SOURCE_FILE})

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 1)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(main)

target_link_libraries(main pico_stdlib pico_runtime hardware_spi hardware_pwm hardware_dma)
