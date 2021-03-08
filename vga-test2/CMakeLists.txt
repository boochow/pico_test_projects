cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD "vgaboard")

include(../pico_sdk_import.cmake)
include(../pico_extras_import.cmake)

project(vgabutton)

pico_sdk_init()

add_executable(vgabutton
  vgabutton.c
  )

target_link_libraries(vgabutton PRIVATE
  pico_stdlib
  pico_scanvideo_dpi)

pico_enable_stdio_usb(vgabutton 1)
pico_enable_stdio_uart(vgabutton 0)

pico_add_extra_outputs(vgabutton)
