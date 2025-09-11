//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#include <string.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "SPIDAC.h"


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
constexpr char SPI_CHANNEL_0_PATH[] = "/dev/spidev0.0";
constexpr char SPI_CHANNEL_1_PATH[] = "/dev/spidev0.1";


/// <summary>
/// Initializes a new instance of class SPIDAC
/// </summary>
SPIDAC::SPIDAC()
{
   open();
}


/// <summary>
/// Releases resources held by the object
/// </summary>
SPIDAC::~SPIDAC()
{
   if (fileDescriptor != -1)
      close(fileDescriptor);
}


/// <summary>
/// opens the file descriptor if it hasn't already been
/// </summary>
void SPIDAC::open()
{
   if (fileDescriptor != -1)
      return;

   fileDescriptor = ::open(SPI_CHANNEL_0_PATH, O_RDWR);
   if (fileDescriptor == -1)
   {
      std::cout << "SPIDAC: open failed" << std::endl;
      return;
   }

   uint8_t mode = 0;
   ioctl(fileDescriptor, SPI_IOC_WR_MODE, &mode);
   ioctl(fileDescriptor, SPI_IOC_RD_MODE, &mode);

   uint8_t bitsPerWord = 8;
   ioctl(fileDescriptor, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord);
   ioctl(fileDescriptor, SPI_IOC_RD_BITS_PER_WORD, &bitsPerWord);

   // 100KHz gets the transaction time down to well under 1ms, which is
   // good for our purposes
   uint32_t frequency = 100000;
   ioctl(fileDescriptor, SPI_IOC_WR_MAX_SPEED_HZ, &frequency);
   ioctl(fileDescriptor, SPI_IOC_RD_MAX_SPEED_HZ, &frequency);
}


/// <summary>
/// sends the given X value
/// </summary>
void SPIDAC::sendX(float x)
{
   sendDacCommand(DacCommand::LoadAAndUpdate, x);
}


/// <summary>
/// sends the given Y value
/// </summary>
void SPIDAC::sendY(float y)
{
   sendDacCommand(DacCommand::LoadBAndUpdate, y);
}


void SPIDAC::sendDacCommand(DacCommand command, float dacValue)
{
   // open if we haven't already
   open();

   // convert the dacValue into an integer from 0 to 1023
   uint16_t iDacValue = 0;
   if (dacValue < 0)
      iDacValue = 0;
   else if (dacValue > 1.0f)
      iDacValue = 1023;
   else
      iDacValue = (uint16_t)(1023 * dacValue + 0.5);

   // make our packet
   uint8_t data[] = {
      (uint8_t)(((uint8_t)command << 4) | (iDacValue >> 6)),
      (uint8_t)(iDacValue << 2)
   };

   struct spi_ioc_transfer spi_message[1];
   memset(spi_message, 0, sizeof(spi_message));
   spi_message[0].tx_buf = (unsigned long)data;
   spi_message[0].len = sizeof(data);

   // on RPi3B this takes about 2ms
   int result = ioctl(fileDescriptor, SPI_IOC_MESSAGE(1), spi_message);
   if (result == -1)
      std::cout << "sendX: ioctl failed" << std::endl;
}
