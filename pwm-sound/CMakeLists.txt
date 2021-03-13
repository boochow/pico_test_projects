cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD "vgaboard")

include(../pico_sdk_import.cmake)
include(../pico_extras_import.cmake)

project(pwm-sound)

pico_sdk_init()


add_executable(pwm-sound
  pwmsound.c
  )

target_link_libraries(pwm-sound PRIVATE
  pico_stdlib
  hardware_pwm
  )

pico_enable_stdio_usb(pwm-sound 1)
pico_enable_stdio_uart(pwm-sound 0)

pico_add_extra_outputs(pwm-sound)
