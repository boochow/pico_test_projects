cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD "vgaboard")

include(../pico_sdk_import.cmake)
include(../pico_extras_import.cmake)

project(multiplane)

pico_sdk_init()


add_executable(multiplane
  multiplane.c
  )

add_compile_definitions(multiplane PRIVATE
  PICO_SCANVIDEO_PLANE_COUNT=2
  PICO_SCANVIDEO_MAX_SCANLINE_BUFFER2_WORDS=10
  )

target_link_libraries(multiplane PRIVATE
  pico_stdlib
  pico_scanvideo_dpi)

pico_enable_stdio_usb(multiplane 1)
pico_enable_stdio_uart(multiplane 0)

pico_add_extra_outputs(multiplane)
