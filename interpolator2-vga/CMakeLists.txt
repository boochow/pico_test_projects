cmake_minimum_required(VERSION 3.12)

include(../pico_sdk_import.cmake)
include(../pico_extras_import.cmake)

project(mode7-vga)

pico_sdk_init()

link_directories(
  pico_scanvideo_dpi
  )

add_executable(
  mode7-vga
  demo.cpp
)

# enable usb output, disable uart output
pico_enable_stdio_usb(mode7-vga 0)
pico_enable_stdio_uart(mode7-vga 1)

# Pull in pico libraries that we need
target_link_libraries(mode7-vga
  pico_stdlib
  hardware_interp
  pico_scanvideo_dpi
  )

# create map/bin/hex file etc.
pico_add_extra_outputs(mode7-vga)
