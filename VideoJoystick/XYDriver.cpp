//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#include "XYDriver.h"


/// <summary>
/// maps the given pixel X and Y to joystick XY
/// </summary>
XY XYDriver::getXY(XY pixelXY)
{
   // map our pixel vector to a sum of our xy01 and xy11 vectors
   float magnitude01, magnitude10, magnitude11;
   decompose(pixelXY - xy00, xy01 - xy00, xy11 - xy00, &magnitude01, &magnitude11);

   // if the magnitude01 value is positive then this is the right set of
   // vectors
   if (magnitude01 > 0)
      return clip(XY(magnitude11, magnitude11 + magnitude01));

   // else we use the other pair
   decompose(pixelXY - xy00, xy10 - xy00, xy11 - xy00, &magnitude10, &magnitude11);
   return clip(XY(magnitude11 + magnitude10, magnitude11));
}


/// <summary>
/// decomposes vin into (aMagnitude*va + bMagnitude*vb)
/// </summary>
void XYDriver::decompose(XY vin, XY va, XY vb, float *aMagnitude, float *bMagnitude)
{
   *aMagnitude =
      determinant(
         vin.x, vb.x,
         vin.y, vb.y
      )
      /
      determinant(
         va.x, vb.x,
         va.y, vb.y
      );

   *bMagnitude =
      determinant(
         va.x, vin.x,
         va.y, vin.y
      )
      /
      determinant(
         va.x, vb.x,
         va.y, vb.y
      );
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


