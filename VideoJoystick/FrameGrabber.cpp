
#include "FrameGrabber.h"

FrameGrabber::FrameGrabber(void)
{
   // Setup preview window defaults
   raspipreview_set_defaults(&preview_parameters);

   // Set up the camera_parameters to default
   raspicamcontrol_set_defaults(&camera_parameters);
}

