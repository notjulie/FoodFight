
#ifndef FRAMEGRABBER_H
#define FRAMEGRABBER_H

#include <stdint.h>
#include <stdio.h>
#include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_connection.h"

extern "C" {
   #include "RaspiCamControl.h"
   #include "RaspiPreview.h"
};


/// Capture/Pause switch method
/// Simply capture for time specified
#define WAIT_METHOD_NONE           0
/// Cycle between capture and pause for times specified
#define WAIT_METHOD_TIMED          1
/// Switch between capture and pause on keypress
#define WAIT_METHOD_KEYPRESS       2
/// Switch between capture and pause on signal
#define WAIT_METHOD_SIGNAL         3
/// Run/record forever
#define WAIT_METHOD_FOREVER        4


class FrameGrabber {
public:
   FrameGrabber(void);

   void EnableCameraCallback(MMAL_PORT_T *camera_video_port);

private:
   static void CameraBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

public:
   int timeout = 5000;                 /// Time taken before frame is grabbed and app then shuts down. Units are milliseconds
   uint32_t width = 640;               /// Requested width of image
   uint32_t height = 480;              /// requested height of image
   int framerate = 90;                 /// Requested frame rate (fps)
   int verbose = 0;                    /// !0 if want detailed run information
   int demoMode = 0;                   /// Run app in demo mode
   int demoInterval = 250;             /// Interval between camera settings changes
   int waitMethod = WAIT_METHOD_NONE;  /// Method for switching between pause and capture

   int onTime = 5000;                  /// In timed cycle mode, the amount of time the capture is on per cycle
   int offTime = 5000;                 /// In timed cycle mode, the amount of time the capture is off per cycle

   int onlyLuma = 0;                   /// Only output the luma / Y plane of the YUV data
   int useRGB = 1;                     /// Output RGB data rather than YUV

   RASPIPREVIEW_PARAMETERS preview_parameters;   /// Preview setup parameters
   RASPICAM_CAMERA_PARAMETERS camera_parameters; /// Camera setup parameters

   MMAL_COMPONENT_T *camera_component = nullptr;    /// Pointer to the camera component
   MMAL_CONNECTION_T *preview_connection = nullptr; /// Pointer to the connection from camera to preview

   MMAL_POOL_T *camera_pool = nullptr;            /// Pointer to the pool of buffers used by camera video port

   int bCapturing = 0;                      /// State of capture/pause

   int cameraNum = 0;                       /// Camera number
   int settings = 0;                        /// Request settings from the camera
   int sensor_mode = 0;                     /// Sensor mode. 0=auto. Check docs/forum for modes selected by other values.

   int frame = 0;
   int64_t starttime = 0;
   int64_t lasttime = 0;

   bool netListen = 0;
};


#endif
