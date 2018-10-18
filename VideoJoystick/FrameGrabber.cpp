
#include <stdexcept>
#include "interface/mmal/mmal_logging.h"
#include "FrameGrabber.h"

FrameGrabber::FrameGrabber(void)
{
   // Setup preview window defaults
   raspipreview_set_defaults(&preview_parameters);

   // Set up the camera_parameters to default
   raspicamcontrol_set_defaults(&camera_parameters);
}

void FrameGrabber::EnableCameraCallback(MMAL_PORT_T *camera_video_port)
{
   if (MMAL_SUCCESS != mmal_port_enable(camera_video_port, CameraBufferCallback))
	  throw std::runtime_error("Failed to setup camera output");
}


/**
 *  buffer header callback function for camera
 *
 *  Callback will dump buffer data to internal buffer
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
void FrameGrabber::CameraBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   // We pass our file handle and other stuff in via the userdata field.

   FrameGrabber *pFrameGrabber = (FrameGrabber *)port->userdata;

   if (pFrameGrabber)
   {
      int bytes_to_write = buffer->length;
      if (pFrameGrabber->onlyLuma)
         bytes_to_write = vcos_min(buffer->length, port->format->es->video.width * port->format->es->video.height);

      if (bytes_to_write)
      {
         mmal_buffer_header_mem_lock(buffer);
         // Do something with the data here
         char bleem[3000];
         memcpy(bleem, buffer->data, 3000);
         //bytes_written = fwrite(buffer->data, 1, bytes_to_write, pData->file_handle);
         mmal_buffer_header_mem_unlock(buffer);

#if 0
// this chunk shows how to grab the timestamp, which is worth looking into
         if (pData->pts_file_handle)
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
               if(pFrameGrabber->frame==0)
            	   pFrameGrabber->starttime=buffer->pts;
               pData->lasttime=buffer->pts;
               pts = buffer->pts - pData->starttime;
               fprintf(pData->pts_file_handle,"%lld.%03lld\n", pts/1000, pts%1000);
               pData->frame++;
            }
         }
#endif
      }
   }
   else
   {
      vcos_log_error("Received a camera buffer callback with no state");
   }

   // release buffer back to the pool
   mmal_buffer_header_release(buffer);

   // and send one back to the port (if still open)
   if (port->is_enabled)
   {
      MMAL_STATUS_T status;

      MMAL_BUFFER_HEADER_T *new_buffer = mmal_queue_get(pFrameGrabber->camera_pool->queue);

      if (new_buffer)
         status = mmal_port_send_buffer(port, new_buffer);

      if (!new_buffer || status != MMAL_SUCCESS)
         vcos_log_error("Unable to return a buffer to the camera port");
   }
}


