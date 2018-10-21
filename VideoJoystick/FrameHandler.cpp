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


void FrameHandler::HandleFrame(std::shared_ptr<VideoFrame> &frame)
{
	int bytes_to_write = frame->GetPixelDataLength();
	int bytes_written = fwrite(frame->GetPixelData(), 1, bytes_to_write, file_handle);
	if (bytes_written != bytes_to_write)
		abort = 1;
}

