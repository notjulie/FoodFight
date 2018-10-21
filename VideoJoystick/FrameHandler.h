/*
 * FrameHandler.h
 *
 *  Created on: Oct 20, 2018
 *      Author: pi
 */

#ifndef FRAMEHANDLER_H_
#define FRAMEHANDLER_H_

#include <stdio.h>
#include <memory>
#include "VideoFrame.h"

class FrameHandler
{
public:
	FrameHandler(void);
	void HandleFrame(std::shared_ptr<VideoFrame> &frame);

public:
	FILE *file_handle = nullptr;                   /// File handle to write buffer data to.
	int abort = 0;                           /// Set to 1 in callback if an error occurs to attempt to abort the capture
};


#endif /* FRAMEHANDLER_H_ */
