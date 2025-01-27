//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#ifndef LIBCAMERA_FRAMEGRABBER_H
#define LIBCAMERA_FRAMEGRABBER_H

#include <mutex>
#include "FrameGrabber.h"
#include "LibCameraManager.h"

class LibCameraFrameGrabber : public FrameGrabber
{
public:
   virtual ~LibCameraFrameGrabber();
   static LibCameraFrameGrabber *createUniqueCamera();

	void startCapturing() override;

private:
   LibCameraFrameGrabber();

   void configureCamera();
   void openUniqueCamera();

private:
   std::mutex mutex;
   std::shared_ptr<libcamera::CameraManager> cameraManager;
   std::shared_ptr<libcamera::Camera> camera;
   std::unique_ptr<libcamera::CameraConfiguration> cameraConfiguration;
   std::unique_ptr<libcamera::FrameBufferAllocator> frameBufferAllocator;
   std::vector<std::unique_ptr<libcamera::Request>> requests;
};

#endif // LIBCAMERA_FRAMEGRABBER_H
