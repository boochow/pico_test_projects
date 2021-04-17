# Sound generator using PWM

A simple sound generator. Three waveforms (square, saw, triangle) + noise, four channels, with independent volume control. Easy to customize.

![Shape](https://raw.githubusercontent.com/boochow/pico_test_projects/images/pwm-sound/pwm-sound.png)

To build:
```
cmake -B build
make -C build
cp build/pwm-sound.uf2 your_pico_drive
```
