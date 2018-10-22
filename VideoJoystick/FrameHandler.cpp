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


void FrameHandler::HandleFrame(const std::shared_ptr<VideoFrame> &frame)
{
	// skip the first several frames until the camera warms up
	++framesReceived;
	if (framesReceived < 100)
		return;

	if (framesWritten >= 20)
		return;

	int bytes_to_write = frame->GetPixelDataLength();
	fwrite(frame->GetPixelData(), 1, bytes_to_write, file_handle);
	++framesWritten;
}

bool FrameHandler::Terminated(void)
{
	return (framesWritten >= 20);
}
