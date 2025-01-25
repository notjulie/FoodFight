//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#ifndef LIBCAMERAMANAGER_H
#define LIBCAMERAMANAGER_H

#include <memory>
#include <libcamera/libcamera.h>

/// <summary>
/// libcmaera lifetime support; get a shared instance to assure that
/// the library is initialized, library will deinitialize when all
/// shared instances go out of scope
/// </summary>
class LibCameraManager final {
public:
   ~LibCameraManager();

   static std::shared_ptr<LibCameraManager> Initialize();

private:
   LibCameraManager();

private:
   libcamera::CameraManager cameraManager;
};



#endif // LIBCAMERAMANAGER_H
