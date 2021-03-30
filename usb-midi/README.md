# USB MIDI message monitor

A USB MIDI device which dumps the all incoming MIDI messages to the UART0.

To build:
```
cmake -B build
make -C build
cp build/usb-midi.uf2 your_pico_drive
```
