cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(demo)

pico_sdk_init()

include_directories(../../pimoroni-pico/libraries/pico_display/)
link_directories(../../pimoroni-pico/build/libraries/pico_graphics/ ../../pimoroni-pico/build/drivers/st7789/)

add_executable(
  demo
  demo.cpp
  ../../pimoroni-pico/libraries/pico_display/pico_display.cpp
)

# enable usb output, disable uart output
pico_enable_stdio_usb(demo 1)
pico_enable_stdio_uart(demo 0)

# Pull in pico libraries that we need
target_link_libraries(demo pico_stdlib  hardware_interp hardware_spi hardware_pwm hardware_dma pico_graphics st7789)

# create map/bin/hex file etc.
pico_add_extra_outputs(demo)
