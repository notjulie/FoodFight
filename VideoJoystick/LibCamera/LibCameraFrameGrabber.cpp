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

   // create our frame processing thread
   frameProcessingThread = new std::thread([this](){ processFrames(); });
}


/// <summary>
/// Releases resources held by the object
/// </summary>
LibCameraFrameGrabber::~LibCameraFrameGrabber()
{
   // shut down our frame processing thread
   terminated = true;
   if (frameProcessingThread != nullptr)
   {
      frameProcessingThread->join();
      delete frameProcessingThread;
      frameProcessingThread = nullptr;
   }

   // this is the official shutdown procedure
   if (camera)
   {
      camera->stop();
      if (frameBufferAllocator)
      {
         frameBufferAllocator->free(cameraConfiguration->at(0).stream());
         frameBufferAllocator.reset();
      }
      camera->release();
      camera.reset();
   }
   cameraManager.reset();
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

   // set our callback
   camera->requestCompleted.connect(requestCompletedCallback);

   int buffersAllocated = frameBufferAllocator->allocate(stream);
   if (buffersAllocated <= 0)
      throw std::runtime_error("LibCameraFrameGrabber::startCapturing: allocate failed");
   std::cout << "Allocated " << buffersAllocated << " buffers" << std::endl;

   libcamera::ControlList controls;
   controls.set(libcamera::controls::FrameDurationLimits, libcamera::Span<const std::int64_t, 2>({11000, 11112}));
   if (0 != camera->start(&controls))
      throw std::runtime_error("LibCameraFrameGrabber::startCapturing: start failed");

   for (int i=0; i<buffersAllocated; ++i)
   {
      std::unique_ptr<libcamera::Request> request = camera->createRequest((uint64_t)this);
      if (!request)
         throw std::runtime_error("LibCameraFrameGrabber::startCapturing: createRequest failed");
      if (0 != request->addBuffer(stream, frameBufferAllocator->buffers(stream)[i].get()))
         throw std::runtime_error("LibCameraFrameGrabber::startCapturing: addBuffer failed");
      if (0 != camera->queueRequest(request.get()))
         throw std::runtime_error("LibCameraFrameGrabber::startCapturing: queueRequest failed");
      requests.push_back(std::move(request));
   }
}


/// <summary>
/// does processing associated with the completion of a request
/// </summary>
void LibCameraFrameGrabber::onRequestCompleted(libcamera::Request *request)
{
   // if the frame was canceled it almost certainly means that we are shutting
   // down so we should proceed to the nearest exit
   if (request->status() == libcamera::Request::RequestCancelled)
      return;

   // some reassuring diagnostics for now
   ++frameCount;
   auto now = std::chrono::steady_clock::now();
   if (now - frameCountReference >= std::chrono::milliseconds(1000))
   {
      std::cout << frameCount << "," << framesProcessed << std::endl;
      frameCount = 0;
      framesProcessed = 0;
      frameCountReference = now;
   }

   // pass it off to the thread that processes requests
   {
      // lock
      std::lock_guard<std::mutex> lock(frameToProcessMutex);

      // if we have a request waiting to be processed we want to requeue it
      // before we replace it with the new request
      if (requestToProcess != nullptr)
      {
         requestToProcess->reuse(libcamera::Request::ReuseBuffers);
         if (0 != camera->queueRequest(requestToProcess))
            throw std::runtime_error("LibCameraFrameGrabber::onRequestCompleted: queueRequest failed");
         requestToProcess = nullptr;
      }

      // post the request to be processed
      requestToProcess = request;

      // trigger the condition
      frameToProcessCondition.notify_all();
   }
}


/// <summary>
/// Thread that processes frames
/// </summary>
void LibCameraFrameGrabber::processFrames()
{
   while (!terminated)
   {
      // wait for a frame to show up
      libcamera::Request *request = nullptr;
      {
         std::unique_lock<std::mutex> lock(frameToProcessMutex);
         frameToProcessCondition.wait(lock, [this](){ return terminated || requestToProcess != nullptr; });
         request = requestToProcess;
         requestToProcess = nullptr;
      }

      // process if we got one
      if (request != nullptr && !terminated)
      {
         ++framesProcessed;

         // dawdle to simulate that we are working really hard
         std::this_thread::sleep_for(std::chrono::milliseconds(100));

         // requeue
         request->reuse(libcamera::Request::ReuseBuffers);
         if (0 != camera->queueRequest(request))
            throw std::runtime_error("LibCameraFrameGrabber::processFrames: queueRequest failed");
      }
   }
}
