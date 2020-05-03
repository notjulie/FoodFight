/*
 * FrameHandler.h
 *
 *  Created on: Oct 20, 2018
 *      Author: pi
 */

#ifndef FRAMEHANDLER_H_
#define FRAMEHANDLER_H_

#include <stdio.h>
#include <deque>
#include <future>
#include <memory>
#include "VideoFrame.h"

class FrameHandler
{
public:
	FrameHandler(void);
	void HandleFrame(const std::shared_ptr<VideoFrame> &frame);
	std::string GetImageAsString(void);

private:
	int framesReceived = 0;
	std::mutex frameRequestMutex;
	std::deque<std::promise<std::string>> frameRequestQueue;
};


#endif /* FRAMEHANDLER_H_ */
