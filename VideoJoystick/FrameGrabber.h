
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


// Forward
class FrameGrabber;

/** Struct used to pass information in camera video port userdata to callback
 */
typedef struct
{
   FILE *file_handle;                   /// File handle to write buffer data to.
   FrameGrabber *pstate;           /// pointer to our state in case required in callback
   int abort;                           /// Set to 1 in callback if an error occurs to attempt to abort the capture
   FILE *pts_file_handle;               /// File timestamps
   int frame;
   int64_t starttime;
   int64_t lasttime;
} PORT_USERDATA;

/** Structure containing all state information for the current run
 */
class FrameGrabber
{
public:
	FrameGrabber(void);
	MMAL_STATUS_T SetupFrameCallback(MMAL_PORT_T *camera_video_port);

private:
	static void CameraBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

public:
   int timeout;                        /// Time taken before frame is grabbed and app then shuts down. Units are milliseconds
   uint32_t width;                          /// Requested width of image
   uint32_t height;                         /// requested height of image
   int framerate;                      /// Requested frame rate (fps)
   char *filename;                     /// filename of output file
   int verbose;                        /// !0 if want detailed run information
   int demoMode;                       /// Run app in demo mode
   int demoInterval;                   /// Interval between camera settings changes
   int waitMethod;                     /// Method for switching between pause and capture

   int onTime;                         /// In timed cycle mode, the amount of time the capture is on per cycle
   int offTime;                        /// In timed cycle mode, the amount of time the capture is off per cycle

   int onlyLuma;                       /// Only output the luma / Y plane of the YUV data
   int useRGB;                         /// Output RGB data rather than YUV

   RASPIPREVIEW_PARAMETERS preview_parameters;   /// Preview setup parameters
   RASPICAM_CAMERA_PARAMETERS camera_parameters; /// Camera setup parameters

   MMAL_COMPONENT_T *camera_component;    /// Pointer to the camera component
   MMAL_CONNECTION_T *preview_connection; /// Pointer to the connection from camera to preview

   MMAL_POOL_T *camera_pool;            /// Pointer to the pool of buffers used by camera video port

   PORT_USERDATA callback_data;         /// Used to move data to the camera callback

   int bCapturing;                      /// State of capture/pause

   int cameraNum;                       /// Camera number
   int settings;                        /// Request settings from the camera
   int sensor_mode;                     /// Sensor mode. 0=auto. Check docs/forum for modes selected by other values.

   int frame;
   char *pts_filename;
   int save_pts;
   int64_t starttime;
   int64_t lasttime;

   bool netListen;
};



#endif
