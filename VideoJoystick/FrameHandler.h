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
	void HandleFrame(const std::shared_ptr<VideoFrame> &frame);
	bool Terminated(void);

public:
	FILE *file_handle = nullptr;                   /// File handle to write buffer data to.

private:
	int framesReceived = 0;
	int framesWritten = 0;
};


#endif /* FRAMEHANDLER_H_ */
