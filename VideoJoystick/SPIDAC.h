
#ifndef SPIDAC_H
#define SPIDAC_H

#include <stdint.h>

// documentation break
//
// RPi 3B pinout:
// 		https://pi4j.com/1.2/pins/model-3b-rev1.html
//
// origin of the SPI class:
// 		https://github.com/milekium/spidev-lib/tree/master/src
//

// RPi 3B SPI device names; they use the same data & clock line
// but different chip select lines
#define SPI_CHANNEL_0_PATH "/dev/spidev0.0"
#define SPI_CHANNEL_1_PATH "/dev/spidev0.1"


/// <summary>
/// Support for a LTC1661 dual 10-bit SPI DAC via the RPi's
/// SPI peripheral
/// </summary>
class SPIDAC
{
public:
   void sendX(uint8_t x);
   void sendY(uint8_t y);
};

#endif
