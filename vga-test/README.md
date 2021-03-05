![Shape](https://raw.githubusercontent.com/boochow/pico_test_projects/images/vga-test/vga_test.jpg)
# Simple VGA signal generator project

This draws a index-colored 40x30 pixel map on the screen.
To build:
`cmake -B build -D"PICO_BOARD=vgaboard"
make -C build
cp build/vga-test.uf2 your_pico_drive`
