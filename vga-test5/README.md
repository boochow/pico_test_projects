# VGA black and white frame buffer

A simple bitmap screen. Hit space bar to switch between the noise-drawing demo and the line-drawing demo.

To build:
```
cmake -B build
make -C build
cp build/framebuf.uf2 your_pico_drive
```
