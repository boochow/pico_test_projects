cmake_minimum_required(VERSION 3.12)

include(../pico_sdk_import.cmake)
include(../pico_extras_import.cmake)

project(vga-test)

pico_sdk_init()


add_executable(vga-test
  vgatest.c
  )

target_link_libraries(vga-test PRIVATE
  pico_stdlib
  pico_scanvideo_dpi)

pico_enable_stdio_usb(vga-test 1)
pico_enable_stdio_uart(vga-test 0)

pico_add_extra_outputs(vga-test)
