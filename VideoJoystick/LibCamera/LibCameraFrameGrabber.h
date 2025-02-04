//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#ifndef LIBCAMERA_FRAMEGRABBER_H
#define LIBCAMERA_FRAMEGRABBER_H

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "FrameGrabber.h"
#include "LibCameraManager.h"
#include "VideoFrame.h"

class LibCameraFrameGrabber : public FrameGrabber
{
public:
   virtual ~LibCameraFrameGrabber();
   static LibCameraFrameGrabber *createUniqueCamera();

	void startCapturing() override;
	void SetupFrameCallback(const std::function<void(const std::shared_ptr<VideoFrame> &)> &callback) override { videoFrameCallback = callback; }

private:
   LibCameraFrameGrabber();

   void configureCamera();
   void onRequestCompleted(libcamera::Request *request);
   void openUniqueCamera();

   void processFrames();

private:
   static void requestCompletedCallback(libcamera::Request *request) { ((LibCameraFrameGrabber*)request->cookie())->onRequestCompleted(request); }

private:
   bool terminated = false;
   std::function<void(const std::shared_ptr<VideoFrame> &)> videoFrameCallback;

   std::shared_ptr<libcamera::CameraManager> cameraManager;
   std::shared_ptr<libcamera::Camera> camera;
   std::unique_ptr<libcamera::CameraConfiguration> cameraConfiguration;
   std::unique_ptr<libcamera::FrameBufferAllocator> frameBufferAllocator;
   std::vector<std::unique_ptr<libcamera::Request>> requests;
   std::vector<std::shared_ptr<VideoFrame>> frames;
   std::chrono::steady_clock::time_point frameCountReference;
   int frameCount = 0;
   int framesProcessed = 0;

   std::thread *frameProcessingThread = nullptr;
   libcamera::Request *requestToProcess = nullptr;
   std::condition_variable frameToProcessCondition;
   std::mutex frameToProcessMutex;
};

#endif // LIBCAMERA_FRAMEGRABBER_H
