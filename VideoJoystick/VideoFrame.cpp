/*
 * VideoFrame.cpp
 *
 *  Created on: Oct 21, 2018
 *      Author: pi
 */


#include "VideoFrame.h"

VideoFrame::VideoFrame(MMAL_BUFFER_HEADER_T *buffer)
{
	// lock the buffer, copy it, unlock
	mmal_buffer_header_mem_lock(buffer);
	pixelData.resize(buffer->length);
	memcpy(&pixelData[0], buffer->data, buffer->length);
	mmal_buffer_header_mem_unlock(buffer);
}


