#!/usr/bin/python

import spidev
import sys

# configure SPI
spi = spidev.SpiDev()
spi.open(0, 0)
spi.max_speed_hz = 7629

# process commands from stdin
channel = 0x9000
value = 0
for line in sys.stdin:
     text = line.rstrip()
     if 'a' == text:
          channel = 0x9000
     elif 'b' == text:
          channel = 0xA000
     else:
          value = channel + (4*int(text))
          msb = value >> 8
          lsb = value & 0xFF
          spi.xfer([msb, lsb])
  





