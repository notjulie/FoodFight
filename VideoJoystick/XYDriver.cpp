//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#include <iostream>
#include "XYDriver.h"


/// <summary>
/// maps the given pixel X and Y to joystick XY
/// </summary>
XY XYDriver::getXY(XY pixelXY, bool verbose)
{
   // journal the new location
   if (journalIndex >= JournalSize)
      journalIndex = 0;
   journal[journalIndex++] = pixelXY;

   // map our pixel vector to a sum of our xy01 and xy11 vectors
   float magnitude01, magnitude10, magnitude11;
   decompose(pixelXY - config.xy00, config.xy01 - config.xy00, config.xy11 - config.xy00, &magnitude01, &magnitude11);
   if (verbose)
   {
      std::cout << (pixelXY - config.xy00).x << "," << (pixelXY - config.xy00).y << std::endl;
      std::cout << (config.xy01 - config.xy00).x << "," << (config.xy01 - config.xy00).y << std::endl;
      std::cout << (config.xy11 - config.xy00).x << "," << (config.xy11 - config.xy00).y << std::endl;
      std::cout << magnitude11 << "," << magnitude01 << std::endl;
   }

   // if the magnitude01 value is positive then this is the right set of
   // vectors
   if (magnitude01 > 0)
      return clip(XY(magnitude11, magnitude11 + magnitude01));

   // else we use the other pair
   decompose(pixelXY - config.xy00, config.xy10 - config.xy00, config.xy11 - config.xy00, &magnitude10, &magnitude11);
   if (verbose)
      std::cout << magnitude11 << "," << magnitude10 << std::endl;
   return clip(XY(magnitude11 + magnitude10, magnitude11));
}


/// <summary>
/// decomposes vin into (aMagnitude*va + bMagnitude*vb)
/// </summary>
void XYDriver::decompose(XY vin, XY va, XY vb, float *aMagnitude, float *bMagnitude)
{
   float denominator =
      determinant(
         va.x, vb.x,
         va.y, vb.y
      );

   *aMagnitude =
      determinant(
         vin.x, vb.x,
         vin.y, vb.y
      )
      /
      denominator;

   *bMagnitude =
      determinant(
         va.x, vin.x,
         va.y, vin.y
      )
      /
      denominator;
}


XY XYDriver::clip(XY xy)
{
   if (xy.x > 1)
      xy.x = 1;
   if (xy.x < 0)
      xy.x = 0;
   if (xy.y > 1)
      xy.y = 1;
   if (xy.y < 0)
      xy.y = 0;
   return xy;
}


float XYDriver::determinant(float a1, float a2, float b1, float b2)
{
   return a1 * b2 - a2 * b1;
}

/// <summary>
/// Returns an XY that represents the average of recent pixel XYs received
/// </summary>
XY XYDriver::getStablePixelXY()
{
   float xSum = 0;
   float ySum = 0;
   for (unsigned i=0; i<JournalSize; ++i)
   {
      xSum += journal[i].x;
      ySum += journal[i].y;
   }

   return XY(xSum / JournalSize, ySum / JournalSize);
}



