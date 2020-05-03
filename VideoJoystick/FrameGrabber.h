
#ifndef FRAMEGRABBER_H
#define FRAMEGRABBER_H

#include <stdint.h>
#include <stdio.h>
#include <functional>
#include <memory>
#include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_connection.h"

extern "C" {
   #include "RaspiCamControl.h"
};

#include "VideoFrame.h"


// Video format information
// 0 implies variable
#define VIDEO_FRAME_RATE_NUM 30
#define VIDEO_FRAME_RATE_DEN 1

// Standard port setting for the camera component
//#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2


/** Structure containing all state information for the current run
 */
class FrameGrabber
{
public:
	FrameGrabber(void);
	MMAL_STATUS_T CreateCameraComponent(void);
	void DisableCamera(void);
	void DestroyCameraComponent(void);
	MMAL_STATUS_T SetupFrameCallback(const std::function<void(const std::shared_ptr<VideoFrame> &)> &callback);

	MMAL_PORT_T *GetVideoPort(void) { return camera_component->output[MMAL_CAMERA_VIDEO_PORT]; }

private:
	void CameraBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

private:
	static void CameraBufferCallbackEntry(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

public:
   uint32_t width;                          /// Requested width of image
   uint32_t height;                         /// requested height of image
   int framerate;                      /// Requested frame rate (fps)

   RASPICAM_CAMERA_PARAMETERS camera_parameters; /// Camera setup parameters

   MMAL_POOL_T *camera_pool;            /// Pointer to the pool of buffers used by camera video port

   int bCapturing;                      /// State of capture/pause

   int cameraNum;                       /// Camera number
   int settings;                        /// Request settings from the camera
   int sensor_mode;                     /// Sensor mode. 0=auto. Check docs/forum for modes selected by other values.

private:
   MMAL_COMPONENT_T *camera_component = nullptr;    /// Pointer to the camera component
   std::function<void(const std::shared_ptr<VideoFrame> &)> frameCallback;
};



#endif
