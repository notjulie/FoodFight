//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#include <mutex>
#include <stdexcept>
#include "LibCameraManager.h"


/// <summary>
/// Initializes the libcamera library
/// </summary>
LibCameraManager::LibCameraManager()
{
   // start the camera manager
   cameraManager.start();
   for (auto const &camera : cameraManager.cameras())
      std::cout << camera->id() << std::endl;
}


/// <summary>
/// Deinitializes the libcamera library
/// </summary>
LibCameraManager::~LibCameraManager()
{
}


/// <summary>
/// Initializes the library and returns a shared pointer to it
/// </summary>
std::shared_ptr<LibCameraManager> LibCameraManager::Initialize()
{
   static std::mutex initMutex;
   static std::weak_ptr<LibCameraManager> globalInstance;

   std::lock_guard<std::mutex> lock(initMutex);
   std::shared_ptr<LibCameraManager> result = globalInstance.lock();
   if (!result)
   {
      result.reset(new LibCameraManager());
      globalInstance = result;
   }

   return result;
}



