//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#ifndef SPIDAC_H
#define SPIDAC_H

#include <stdint.h>


/// <summary>
/// Support for a LTC1661 dual 10-bit SPI DAC via the RPi's
/// SPI peripheral
/// </summary>
class SPIDAC
{
public:
   SPIDAC();
   ~SPIDAC();

   void sendX(float x);
   void sendY(float y);

private:
   enum class DacCommand : uint8_t {
      NoOp = 0,
      LoadANoUpdate = 1,
      LoadBNoUpdate = 2,
      Reserved3 = 3,
      Reserved4 = 4,
      Reserved5 = 5,
      Reserved6 = 6,
      Reserved7 = 7,
      UpdateOutputs = 8,
      LoadAAndUpdate = 9,
      LoadBAndUpdate = 10,
      Reserved11 = 11,
      Reserved12 = 12,
      Wake = 13,
      Sleep = 14,
      UpdateAndWake = 15
   };

private:
   void open();
   void sendDacCommand(DacCommand command, float dacValue);

private:
   int fileDescriptor = -1;
};

#endif
