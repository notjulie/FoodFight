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
   virtual ~LibCameraFrameGrabber();

   static LibCameraFrameGrabber *createUniqueCamera();

private:
   LibCameraFrameGrabber();
   void openUniqueCamera();

private:
   std::shared_ptr<libcamera::CameraManager> cameraManager;
   std::shared_ptr<libcamera::Camera> camera;
};

#endif // LIBCAMERA_FRAMEGRABBER_H
