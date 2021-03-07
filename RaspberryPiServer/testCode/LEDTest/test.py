#!/usr/bin/env python3

import board
import neopixel
from time import sleep

pixels = neopixel.NeoPixel(board.D12, 7)

pixels[0] = (255, 0, 0)
pixels.show()

sleep(10)

print("quitting")


