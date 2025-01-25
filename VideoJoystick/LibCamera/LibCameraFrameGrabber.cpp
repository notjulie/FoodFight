//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#include "LibCameraFrameGrabber.h"


LibCameraFrameGrabber::LibCameraFrameGrabber()
{
   // initialize libcamera
   cameraManager = LibCameraManager::Initialize();
}


LibCameraFrameGrabber::~LibCameraFrameGrabber()
{
}
