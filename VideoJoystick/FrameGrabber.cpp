
#include <stdexcept>
#include "interface/mmal/mmal_logging.h"
#include "FrameGrabber.h"


FrameGrabber::FrameGrabber(void)
{
}

MMAL_STATUS_T FrameGrabber::SetupFrameCallback(MMAL_PORT_T *camera_video_port)
{
    return mmal_port_enable(camera_video_port, CameraBufferCallback);
}


void FrameGrabber::CameraBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   MMAL_BUFFER_HEADER_T *new_buffer;

   // We pass our file handle and other stuff in via the userdata field.

   PORT_USERDATA *pData = (PORT_USERDATA *)port->userdata;

   if (pData)
   {
      int bytes_written = 0;
      int bytes_to_write = buffer->length;

      if (pData->pstate->onlyLuma)
         bytes_to_write = vcos_min(buffer->length, port->format->es->video.width * port->format->es->video.height);

      vcos_assert(pData->file_handle);

      if (bytes_to_write)
      {
         mmal_buffer_header_mem_lock(buffer);
         bytes_written = fwrite(buffer->data, 1, bytes_to_write, pData->file_handle);
         mmal_buffer_header_mem_unlock(buffer);

         if (bytes_written != bytes_to_write)
         {
            vcos_log_error("Failed to write buffer data (%d from %d)- aborting", bytes_written, bytes_to_write);
            pData->abort = 1;
         }
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
               if(pData->pstate->frame==0)
                  pData->pstate->starttime=buffer->pts;
               pData->lasttime=buffer->pts;
               pts = buffer->pts - pData->starttime;
               fprintf(pData->pts_file_handle,"%lld.%03lld\n", pts/1000, pts%1000);
               pData->frame++;
            }
         }
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

      new_buffer = mmal_queue_get(pData->pstate->camera_pool->queue);

      if (new_buffer)
         status = mmal_port_send_buffer(port, new_buffer);

      if (!new_buffer || status != MMAL_SUCCESS)
         vcos_log_error("Unable to return a buffer to the camera port");
   }
}
