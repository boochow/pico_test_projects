# USB MIDI message monitor

A USB MIDI device which dumps the all incoming MIDI messages to the UART0.

![Shape](https://raw.githubusercontent.com/boochow/pico_test_projects/images/usb-midi/usb-midi-monitor.png)

To build:
```
cmake -B build
make -C build
cp build/usb-midi.uf2 your_pico_drive
```
