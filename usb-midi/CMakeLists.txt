cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD "pico")

include(../pico_sdk_import.cmake)
include(../pico_extras_import.cmake)

project(usb-midi)

pico_sdk_init()


add_executable(usb-midi
  usb_midi.c
  usb_descriptors.c
  )

target_include_directories(usb-midi PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  )

target_link_libraries(usb-midi PRIVATE
  pico_stdlib
  hardware_pwm
  tinyusb_device
  tinyusb_board
  )

pico_enable_stdio_usb(usb-midi 0)
pico_enable_stdio_uart(usb-midi 1)

pico_add_extra_outputs(usb-midi)
