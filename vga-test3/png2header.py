import os, sys, math
from PIL import Image
import numpy as np

if len(sys.argv) != 2:
    sys.stderr.write("png2header imagefile\n")
    sys.exit(1)

if not os.path.exists(sys.argv[1]):
    sys.stderr.write(sys.argv[1] + " not found\n")
    sys.exit(1)

im = Image.open(sys.argv[1])
if not((im.size[0] <= 640) and (im.size[1] <= 480)):
    sys.stderr.write("width and height of the image must be smaller than 640x480\n")
    sys.exit(1)

print("const int image_width  = ", end='')
print(im.size[0], end='')
print(";")

print("const int image_height = ", end='')
print(im.size[1], end='')
print(";\n")

indexed = np.array(im)
print('const uint16_t image[] = \n{')
for l in indexed:
    print("\t", end='')
    for p in l:
        color = p[0]>>3 | (p[1] & 0xfc)<<3 | (p[2] & 0xf8)<<8
        print(hex(color), ', ', sep='', end='')
    print()
print('};')

