![Shape](https://raw.githubusercontent.com/boochow/pico_test_projects/images/vga-test5/bw-framebuf.jpg)
# VGA black and white frame buffer

A simple bitmap screen. Hit space bar to switch between the noise-drawing demo and the line-drawing demo.

Both the two core version (framebuf) and the single core version (framebuf-single) are included.

To build:
```
cmake -B build
make -C build
cp build/framebuf.uf2 your_pico_drive
```
