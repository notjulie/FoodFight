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


struct XYDriverConfig {
   XY xy00;
   XY xy01 = {0, 480};
   XY xy10 = {640, 0};
   XY xy11 = {640, 480};
};

/// <summary>
/// Manages converting pixel locations to XY joystick outputs
/// </summary>
class XYDriver {
public:
   XYDriverConfig getConfig() const { return config; }
   void setConfig(const XYDriverConfig &config) { this->config = config; }

   XY getXY(XY pixelXY, bool verbose = false);

   void cal00() { config.xy00 = getStablePixelXY(); }
   void cal01() { config.xy01 = getStablePixelXY(); }
   void cal10() { config.xy10 = getStablePixelXY(); }
   void cal11() { config.xy11 = getStablePixelXY(); }

private:
   XY clip(XY xy);
   void decompose(XY vin, XY va, XY vb, float *aMagnitude, float *bMagnitude);
   float determinant(float a1, float a2, float b1, float b2);
   XY getStablePixelXY();

private:
   static constexpr unsigned JournalSize = 60;

   XYDriverConfig config;

   XY journal[JournalSize];
   unsigned journalIndex = 0;
};

#endif
