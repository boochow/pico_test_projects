cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD "vgaboard")

include(../pico_sdk_import.cmake)
include(../pico_extras_import.cmake)

project(vga-image)

pico_sdk_init()


add_executable(vga-image
  vgaimage.c
  )

add_executable(vga-image-dma
  vgaimage-dma.c
  )

target_link_libraries(vga-image PRIVATE
  pico_stdlib
  pico_scanvideo_dpi)

target_link_libraries(vga-image-dma PRIVATE
  pico_stdlib
  pico_scanvideo_dpi)

target_compile_definitions(vga-image-dma PRIVATE
  PICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=500
  )

pico_enable_stdio_usb(vga-image 1)
pico_enable_stdio_uart(vga-image 0)
pico_enable_stdio_usb(vga-image-dma 0)
pico_enable_stdio_uart(vga-image-dma 1)

pico_add_extra_outputs(vga-image)
pico_add_extra_outputs(vga-image-dma)
