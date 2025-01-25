//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#include <mutex>
#include "LibCameraManager.h"


/// <summary>
/// Initializes the library and returns a shared pointer to it; the
/// documenation generally implies that it should remain for the
/// duration of the app.  We keep a shared pointer to it, so the object
/// won't be deleted until our global instance is closed at app shutdown
/// </summary>
std::shared_ptr<libcamera::CameraManager> LibCameraManager::Initialize()
{
   static std::shared_ptr<libcamera::CameraManager> cameraManager;

   static std::once_flag once;
   std::call_once(once, [](){
      cameraManager.reset(new libcamera::CameraManager());
      cameraManager->start();
      for (auto const &camera : cameraManager->cameras())
         std::cout << camera->id() << std::endl;
   });

   return cameraManager;
}

