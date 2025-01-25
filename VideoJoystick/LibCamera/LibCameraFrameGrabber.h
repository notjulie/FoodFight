//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#ifndef LIBCAMERA_FRAMEGRABBER_H
#define LIBCAMERA_FRAMEGRABBER_H

#include "FrameGrabber.h"
#include "LibCameraManager.h"

class LibCameraFrameGrabber : public FrameGrabber
{
public:
   LibCameraFrameGrabber();
   virtual ~LibCameraFrameGrabber();

private:
   std::shared_ptr<libcamera::CameraManager> cameraManager;
};

#endif // LIBCAMERA_FRAMEGRABBER_H
