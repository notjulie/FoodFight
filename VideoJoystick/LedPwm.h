//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#ifndef LEDPWM_H
#define LEDPWM_H


class LedPwm {
private:
   // single instance; constructor is private
   LedPwm();
   ~LedPwm();

public:
   static LedPwm *getInstance();

   void setDutyCycle(float dutyCycle);
};


#endif
