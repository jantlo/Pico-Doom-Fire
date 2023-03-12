
# requires to install PIL -> execute: pip install pillow

from PIL import Image
from sys import stdout

#img = Image.open("doom256.png")
img = Image.open("doom.png")

print("// This file has been automatically generated using the convert.py tool")
print("")

print('#include "Utils.h"')
print("")
print("namespace doom_sprite")
print("{")
print(f"constexpr uint32_t k_width{{{img.width}}};")
print(f"constexpr uint32_t k_height{{{img.height}}};")
print("")

palette = img.getpalette()
if palette is None:
    print("static constexpr rgb565_t colors[]{")
    stdout.write("\t")
    data = img.getdata()
    for i in range(len(data)):
        stdout.write(f"to565{data[i]}, ")
        if (i + 1) % 5 == 0:
            stdout.write("\n\t")
    print("\n};")
else:
    entries_count = int(len(palette) / 3)
    print("static constexpr rgb565_t palette565[]{")
    stdout.write("\t")
    for i in range(entries_count):
        stdout.write(f"to565({palette[i*3]}, {palette[i*3+1]},{palette[i*3+2]}), ")
        if (i + 1) % 6 == 0:
            stdout.write("\n\t")
    print("};")
    print("")

    print("\nstatic constexpr extended_rgb565_t paletteExtended565[]{")
    stdout.write("\t")
    for i in range(entries_count):
        stdout.write(f"toExtended565({palette[i*3]}, {palette[i*3+1]},{palette[i*3+2]}), ")
        if (i + 1) % 4 == 0:
            stdout.write("\n\t")
    print("};")
    print("")
    print("static constexpr uint8_t indexedColors[]{")
    data = img.getdata()
    stdout.write("\t")
    for i in range(len(data)):
        stdout.write(f"{hex(data[i])}, ")
        if (i+1) % 24 == 0:
            stdout.write("\n\t")
    print("\n};")
print("}")