
#include <stdexcept>
#include "interface/mmal/mmal_logging.h"
#include "FrameGrabber.h"


FrameGrabber::FrameGrabber(void)
{
}

MMAL_STATUS_T FrameGrabber::SetupFrameCallback(const std::function<void(const std::shared_ptr<VideoFrame> &)> &callback)
{
	this->frameCallback = callback;
	GetVideoPort()->userdata = (struct MMAL_PORT_USERDATA_T *)this;
    return mmal_port_enable(GetVideoPort(), CameraBufferCallbackEntry);
}


void FrameGrabber::CameraBufferCallbackEntry(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   FrameGrabber *pThis = (FrameGrabber *)port->userdata;
   if (pThis != nullptr)
	   pThis->CameraBufferCallback(port, buffer);
}

void FrameGrabber::CameraBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	// create a VideoFrame from the buffer; this way the frame handler can do whatever
	// it wants, including processing it later on a different thread if it so desires
	std::shared_ptr<VideoFrame> frame;
	frame.reset(new VideoFrame(buffer));

	// pass the buffer to the frame handler
	frameCallback(frame);

	// release buffer back to the pool
	mmal_buffer_header_release(buffer);

	// and send one back to the port (if still open)
	if (port->is_enabled)
	{
	  MMAL_STATUS_T status;

	  MMAL_BUFFER_HEADER_T *new_buffer = mmal_queue_get(camera_pool->queue);

	  if (new_buffer)
		 status = mmal_port_send_buffer(port, new_buffer);

	  if (!new_buffer || status != MMAL_SUCCESS)
		 vcos_log_error("Unable to return a buffer to the camera port");
	}
}
