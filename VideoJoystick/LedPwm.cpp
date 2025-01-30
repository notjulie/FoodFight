//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#include <wiringPi.h>
#include "LedPwm.h"

// for some reason 18 means PWM0
constexpr int PWM_PIN_NUMBER = 18;

// for RPi3 this is the undivided clock
constexpr int PWM_MASTER_CLOCK = 19200000;

// our desired PWM frequency
constexpr int PWM_FREQUENCY = 1000;

constexpr int PWM_RANGE = 100000;


/// <summary>
/// Initializes a new instance of class LedPwm
/// </summary>
LedPwm::LedPwm()
{
   // initialize wiringPi
   wiringPiSetupGpio();

   // initialize our pin
   pinMode(PWM_PIN_NUMBER, PWM_OUTPUT);
   pwmSetClock(PWM_MASTER_CLOCK / PWM_FREQUENCY);
   pwmSetRange(PWM_RANGE);

   // set our initial state
   setDutyCycle(0.5);
}


/// <summary>
/// Releases resources held by the object
/// </summary>
LedPwm::~LedPwm()
{
   // leave the pin floating on exit
   pinMode(PWM_PIN_NUMBER, INPUT);
}


/// <summary>
/// gets the global singleton
/// </summary>
LedPwm *LedPwm::getInstance()
{
   static LedPwm instance;
   return &instance;
}


/// <summary>
/// Sets the duty cycle
/// </summary>
void LedPwm::setDutyCycle(float dutyCycle)
{
   pwmWrite(PWM_PIN_NUMBER, (int)(PWM_RANGE * dutyCycle));
}
