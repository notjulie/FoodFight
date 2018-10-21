/*
 * FrameHandler.cpp
 *
 *  Created on: Oct 21, 2018
 *      Author: pi
 */

#include "FrameHandler.h"

FrameHandler::FrameHandler(void)
{
}


void FrameHandler::HandleFrame(MMAL_BUFFER_HEADER_T *buffer)
{
	int bytes_to_write = buffer->length;
	int bytes_written = fwrite(buffer->data, 1, bytes_to_write, file_handle);

	if (bytes_written != bytes_to_write)
	{
		abort = 1;
	}

	if (pts_file_handle)
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
		   lasttime = buffer->pts;
		   pts = buffer->pts - starttime;
		   fprintf(pts_file_handle,"%lld.%03lld\n", pts/1000, pts%1000);
		   frame++;
		}
	}
}

