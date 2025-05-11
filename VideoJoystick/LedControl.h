//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#ifndef LEDCONTROL_H
#define LEDCONTROL_H


/// <summary>
/// Class for controlling the LED output; I tried it as a PWM, but
/// it turned out there was no value in doing anything other than
/// full on... the LED intensity is about right that way.
/// </summary>
class LedControl {
private:
   // single instance; constructor is private
   LedControl();
   ~LedControl();

public:
   static LedControl *getInstance();

   void setLedOn(bool on);
};


#endif
