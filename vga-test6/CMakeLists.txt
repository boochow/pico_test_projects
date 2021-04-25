cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD "vgaboard")

include(../pico_sdk_import.cmake)
include(../pico_extras_import.cmake)

project(scanline)

pico_sdk_init()


add_executable(scanline
  scanline.c
  )

target_link_libraries(scanline PRIVATE
  pico_stdlib
  pico_scanvideo_dpi)

pico_enable_stdio_usb(scanline 1)
pico_enable_stdio_uart(scanline 0)

pico_add_extra_outputs(scanline)
