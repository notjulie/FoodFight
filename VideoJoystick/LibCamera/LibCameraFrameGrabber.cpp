//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#include "LibCameraFrameGrabber.h"


/// <summary>
/// Initializes a new instance of class LibCameraFrameGrabber
/// </summary>
LibCameraFrameGrabber::LibCameraFrameGrabber()
{
   // initialize libcamera
   cameraManager = LibCameraManager::Initialize();
}


/// <summary>
/// Releases resources held by the object
/// </summary>
LibCameraFrameGrabber::~LibCameraFrameGrabber()
{
}


/// <summary>
/// Creates an instance assuming that there is exactly one camera device on
/// the system; fails if there is not a unique camera
/// </summary>
LibCameraFrameGrabber *LibCameraFrameGrabber::createUniqueCamera()
{
   std::unique_ptr<LibCameraFrameGrabber> camera(new LibCameraFrameGrabber());
   camera->openUniqueCamera();
   return camera.release();
}

/// <summary>
/// Opens the only camera on the system; fails if there is not a unique camera
/// <summary>
void LibCameraFrameGrabber::openUniqueCamera()
{
   // examine the camera list
   auto cameras = cameraManager->cameras();
   if (cameras.size() == 0)
      throw std::runtime_error("LibCameraFrameGrabber::openUniqueCamera: no cameras");
   if (cameras.size() != 1)
      throw std::runtime_error("LibCameraFrameGrabber::openUniqueCamera: multiple cameras");

   // get the real camera instance; the documentation says to watch for null, as this may not
   // be the same instance as what's in the list above
   this->camera = cameraManager->get(cameras[0]->id());
   if (!this->camera)
      throw std::runtime_error("LibCameraFrameGrabber::openUniqueCamera: error getting camera");
   this->camera->acquire();
}
