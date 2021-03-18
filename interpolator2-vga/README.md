![Shape](https://raw.githubusercontent.com/boochow/pico_test_projects/images/interpolator2/demo-vga.jpg)
# interpolator demo similar to SNES mode7
This demo uses two interpolators. One for picking a tile to be drawn and another for picking the color from the appropriate tile data.

This demo is written for VGA demo board.

# controls
Button A: turn left

Button B: thrust

Button C: turn right

Button A+C: jump

# making your original map data
The map data is a byte array. Each byte holds an index number of `tiles`.

`png2header.py` is a tool to generate `map.h` from a PNG file. `map.png`, which I made from Pico's pinout diagram, is the source of the original `map.h`.

With `png2header.py`, you can generate your original map data from an image file.

The file must be in 8-bit index color format and the image's width and height both must be a power of two.
Also, you need to prepare tile data `tiles[][256]` corresponding to the number of colors you used in the image.