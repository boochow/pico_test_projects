import os
import sys
import math
from PIL import Image
import numpy as np


def is_power_of_two(x):
    return (x & (x - 1)) == 0

if len(sys.argv) != 2:
    sys.stderr.write("png2header imagefile\n")
    sys.exit(1)

if not os.path.exists(sys.argv[1]):
    sys.stderr.write(sys.argv[1] + " not found\n")
    sys.exit(1)

im = Image.open(sys.argv[1])
if not(is_power_of_two(im.size[0]) and is_power_of_two(im.size[1])):
    sys.stderr.write("width and height of the image must be 2^x\n")
    sys.exit(1)

print("constexpr uint8_t map_width  = ", end='')
print(im.size[0].bit_length() - 1, end='')
print(";")

print("constexpr uint8_t map_height = ", end='')
print(im.size[1].bit_length() - 1, end='')
print(";\n")

indexed = np.array(im)
print('constexpr uint8_t map[] = \n{')
for l in indexed:
    print("\t", end='')
    for p in l:
        print(p, ', ', sep='', end='')
    print()
print('};')
