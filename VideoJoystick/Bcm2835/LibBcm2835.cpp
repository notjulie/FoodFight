//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#include <bcm2835.h>
#include <mutex>
#include <stdexcept>
#include "bcm_host.h"

#include "LibBcm2835.h"


/// <summary>
/// Initializes the Bcm2835 library
/// </summary>
LibBcm2835::LibBcm2835()
{
	// initialize access to the GPU
	bcm_host_init();

	// initialize the bcm2835 library
	if (!bcm2835_init())
	{
		throw std::runtime_error("Error initializing bcm2835 library");
	}

	bcm2835_pwm_set_data(18, BCM2835_GPIO_FSEL_ALT5);

	bcm2835_pwm_set_clock(1000);
	bcm2835_pwm_set_mode(0,1,1);
	bcm2835_pwm_set_range(0,1024);
	bcm2835_pwm_set_data(0,300);
}


/// <summary>
/// Deinitializes the Bcm2835 library
/// </summary>
LibBcm2835::~LibBcm2835()
{
	bcm2835_close();
}


/// <summary>
/// Initializes the library and returns a shared pointer to it
/// </summary>
std::shared_ptr<LibBcm2835> LibBcm2835::Initialize()
{
   static std::mutex initMutex;
   static std::weak_ptr<LibBcm2835> globalInstance;

   std::lock_guard<std::mutex> lock(initMutex);
   std::shared_ptr<LibBcm2835> result = globalInstance.lock();
   if (!result)
   {
      result.reset(new LibBcm2835());
      globalInstance = result;
   }

   return result;
}


