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

   uint32_t frequency = 10000;
   ioctl(fileDescriptor, SPI_IOC_WR_MAX_SPEED_HZ, &frequency);
   ioctl(fileDescriptor, SPI_IOC_RD_MAX_SPEED_HZ, &frequency);
}


/// <summary>
/// sends the given X value
/// </summary>
void SPIDAC::sendX(uint8_t x)
{
   open();

   uint8_t data[] = {0x55};

   struct spi_ioc_transfer spi_message[1];
   memset(spi_message, 0, sizeof(spi_message));
   spi_message[0].tx_buf = (unsigned long)data;
   spi_message[0].len = 1;

   int result = ioctl(fileDescriptor, SPI_IOC_MESSAGE(1), spi_message);
   if (result == -1)
      std::cout << "sendX: ioctl failed" << std::endl;
   else
      std::cout << "sendX: ioctl succeeded" << std::endl;
}


void SPIDAC::sendY(uint8_t y)
{
   if (y == 0)
      y = 1;
}
