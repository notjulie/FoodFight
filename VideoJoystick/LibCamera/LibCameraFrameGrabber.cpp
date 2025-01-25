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

   // configure
   configureCamera();
}


/// <summary>
/// configure the camera the way that we expect it
/// </summary>
void LibCameraFrameGrabber::configureCamera()
{
   // grab the default configuration for raw frame capture
   cameraConfiguration = camera->generateConfiguration({ libcamera::StreamRole::Raw });
   if (cameraConfiguration->size() != 1)
      throw std::runtime_error("LibCameraFrameGrabber::configureCamera: expected single stream configuration");
   auto &config = cameraConfiguration->at(0);

   // set our desired image size; the RPi camera has its best frame rate at this
   // resolution, 90 FPS for v1 camera, >200 for some of the newer ones
   config.size.width = 640;
   config.size.height = 480;
   config.bufferCount = 10;
   cameraConfiguration->validate();
   std::cout << cameraConfiguration->at(0).toString() << std::endl;

   if (0 != camera->configure(cameraConfiguration.get()))
      throw std::runtime_error("LibCameraFrameGrabber::configureCamera: configure failed");
}


/// <summary>
/// begin capturing images
/// </summary>
void LibCameraFrameGrabber::startCapturing()
{
   libcamera::Stream *stream = cameraConfiguration->at(0).stream();

   // create our buffer allocator
   frameBufferAllocator.reset(new libcamera::FrameBufferAllocator(camera));

   int buffersAllocated = frameBufferAllocator->allocate(stream);
   if (buffersAllocated <= 0)
      throw std::runtime_error("LibCameraFrameGrabber::startCapturing: allocate failed");
   std::cout << "Allocated " << buffersAllocated << " buffers" << std::endl;

   for (int i=0; i<buffersAllocated; ++i)
   {
      std::unique_ptr<libcamera::Request> request = camera->createRequest();
      if (!request)
         throw std::runtime_error("LibCameraFrameGrabber::startCapturing: createRequest failed");
      if (0 != request->addBuffer(stream, frameBufferAllocator->buffers(stream)[i].get()))
         throw std::runtime_error("LibCameraFrameGrabber::startCapturing: addBuffer failed");
      requests.push_back(std::move(request));
   }
}


