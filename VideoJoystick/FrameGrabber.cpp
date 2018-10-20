
#include <stdexcept>
#include "interface/mmal/mmal_logging.h"
#include "FrameGrabber.h"


FrameGrabber::FrameGrabber(void)
{
}

MMAL_STATUS_T FrameGrabber::SetupFrameCallback(FrameHandler *frameHandler)
{
	this->frameHandler = frameHandler;
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
	MMAL_BUFFER_HEADER_T *new_buffer;

	// We pass our file handle and other stuff in via the userdata field.
	int bytes_written = 0;
	int bytes_to_write = buffer->length;

	if (onlyLuma)
	 bytes_to_write = vcos_min(buffer->length, port->format->es->video.width * port->format->es->video.height);

	vcos_assert(frameHandler->file_handle);

	if (bytes_to_write)
	{
	 mmal_buffer_header_mem_lock(buffer);
	 bytes_written = fwrite(buffer->data, 1, bytes_to_write, frameHandler->file_handle);
	 mmal_buffer_header_mem_unlock(buffer);

	 if (bytes_written != bytes_to_write)
	 {
		vcos_log_error("Failed to write buffer data (%d from %d)- aborting", bytes_written, bytes_to_write);
		frameHandler->abort = 1;
	 }
	 if (frameHandler->pts_file_handle)
	 {
		// Every buffer should be a complete frame, so no need to worry about
		// fragments or duplicated timestamps. We're also in RESET_STC mode, so
		// the time on frame 0 should always be 0 anyway, but simply copy the
		// code from raspivid.
		// MMAL_TIME_UNKNOWN should never happen, but it'll corrupt the timestamps
		// file if saved.
		if(buffer->pts != MMAL_TIME_UNKNOWN)
		{
		   int64_t pts;
		   if(frame==0)
			  starttime=buffer->pts;
		   frameHandler->lasttime=buffer->pts;
		   pts = buffer->pts - frameHandler->starttime;
		   fprintf(frameHandler->pts_file_handle,"%lld.%03lld\n", pts/1000, pts%1000);
		   frameHandler->frame++;
		}
	 }
	}

	// release buffer back to the pool
	mmal_buffer_header_release(buffer);

	// and send one back to the port (if still open)
	if (port->is_enabled)
	{
	  MMAL_STATUS_T status;

	  new_buffer = mmal_queue_get(camera_pool->queue);

	  if (new_buffer)
		 status = mmal_port_send_buffer(port, new_buffer);

	  if (!new_buffer || status != MMAL_SUCCESS)
		 vcos_log_error("Unable to return a buffer to the camera port");
	}
}
