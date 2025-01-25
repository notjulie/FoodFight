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
/// shared instances go out of scope, i.e. at application shutdown
/// </summary>
class LibCameraManager final {
public:
   static std::shared_ptr<libcamera::CameraManager> Initialize();

private:
   LibCameraManager() = delete;
   ~LibCameraManager() = delete;
};



#endif // LIBCAMERAMANAGER_H
