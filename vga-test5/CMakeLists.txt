cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD "vgaboard")

include(../pico_sdk_import.cmake)
include(../pico_extras_import.cmake)

project(framebuf)

pico_sdk_init()


add_executable(framebuf
  framebuf.c
  )

add_compile_definitions(framebuf PRIVATE
  PICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=350
  )

target_link_libraries(framebuf PRIVATE
  pico_stdlib
  pico_scanvideo_dpi)

pico_enable_stdio_usb(framebuf 1)
pico_enable_stdio_uart(framebuf 0)

pico_add_extra_outputs(framebuf)
