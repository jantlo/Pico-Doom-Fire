# Doom Pico Fire
[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/6V3TeqOpLDg/0.jpg)](https://www.youtube.com/watch?v=6V3TeqOpLDg)

## What it this
It is just a simple implementation of the famous and nice looking Doom's fire found on for the PSX and N64 Doom ports. For more info just visit the Fabien Sanglard awesome blog: https://fabiensanglard.net/doom_fire_psx/

The goal is just to get familiar with the Raspberry PI pico hardware and its SDK. 

## Hardware
The hardware used for this demo is:
* Raspberry PI pico
* the Pico LCD Waveshare LCD 1.3: It provide the LCD + 9 buttons (4 cursors + Ctrl, A, B, X, Y buttons), link: https://www.waveshare.com/wiki/Pico-LCD-1.3

The LCD is controlled via the ST7789VW.

## How to build it
Like other Raspberry PI pico projects, it has a CMakeFile.txt, so set the PICO_SDK_PATH and run cmake inside the build folder for instance. If the build success a main.uf2 file should be created, just enable pico bootloader which will present on PC as usb mass storage and copy the main.uf2 into its root folder. The pico will reboot and the fire effect will start automatically.

## How to use
The buttons A, B, X, Y can do some actions:
* A: Start/Stop fire.
* B: Jump to the next effect.
* X: Restart.
* Y: Enters into bootloader.

## Some insides
Where is the framebuffer? There is no framebuffer, instead each scanline is produced and sent to to the LCD controller. The transfer is done using the DMA so while sending the current scanline the next one can be prepared by the CPU. In case the CPU finishes earlier than the DMA then it will wait till the DMA is ready.

## How to test it
If you have already got the hardware you can test it building it or just flashing one of the uf2 files inside the bin folder:
* ``main.uf2``: It is permanent, save it to flash.
* ``main_noflash.uf2``: It is saved to RAM and it will alive while the board is power.

## How to generate the Doom's logo sprite.
It is already generated and stored inside the media folder repo but in case you want to change it, execute the python script inside the doc folder:
 ````
 python3 ./convert.py > ../src/DoomSprite.h
 ````
It will exported to 16bits RGB565.

