//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#include <cstdlib>
#include "LedControl.h"


/// <summary>
/// Initializes a new instance of class LedControl
/// </summary>
LedControl::LedControl()
{
   // for our purposes there's no reason to be more complicated
   // than calling the command line gpio commmand
   std::system("gpio mode 1 out");
   setLedOn(false);
}


/// <summary>
/// Releases resources held by the object
/// </summary>
LedControl::~LedControl()
{
   // leave the pin floating on exit
   std::system("gpio mode 1 in");
}


/// <summary>
/// gets the global singleton
/// </summary>
LedControl *LedControl::getInstance()
{
   static LedControl instance;
   return &instance;
}


/// <summary>
/// Sets the LED state
/// </summary>
void LedControl::setLedOn(bool on)
{
   // the LED control is active low
   std::system(on ? "gpio write 1 0" : "gpio write 1 1");
}
