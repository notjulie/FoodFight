//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#ifndef XYDRIVER_H
#define XYDRIVER_H

/// <summary>
/// simple point/vector class
/// </summary>
struct XY {
public:
   float x = 0;
   float y = 0;

   XY() = default;
   XY(float _x, float _y) : x(_x), y(_y) {}

   XY operator-(const XY &xy) {
      return XY(x - xy.x, y - xy.y);
   }
};


/// <summary>
/// Manages converting pixel locations to XY joystick outputs
/// </summary>
class XYDriver {
public:
   XY getXY(XY pixelXY, bool verbose = false);

   void cal00() { xy00 = getStablePixelXY(); }
   void cal01() { xy01 = getStablePixelXY(); }
   void cal10() { xy10 = getStablePixelXY(); }
   void cal11() { xy11 = getStablePixelXY(); }

private:
   XY clip(XY xy);
   void decompose(XY vin, XY va, XY vb, float *aMagnitude, float *bMagnitude);
   float determinant(float a1, float a2, float b1, float b2);
   XY getStablePixelXY();

private:
   static constexpr unsigned JournalSize = 60;

   XY xy00;
   XY xy01 = {0, 480};
   XY xy10 = {640, 0};
   XY xy11 = {640, 480};

   XY journal[JournalSize];
   unsigned journalIndex = 0;
};

#endif
