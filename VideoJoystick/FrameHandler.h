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

/// <summary>
/// Processes incoming frames, reports calculated XY position of red dot
/// </summary>
class FrameHandler
{
public:
	FrameHandler();
	void HandleFrame(const std::shared_ptr<VideoFrame> &frame);
	std::string GetImageAsString();
   double getSaturiationPercent() const { return saturationPercent; }

   int getX() const { return currentX; }
   int getY() const { return currentY; }

private:
	int framesReceived = 0;
	double saturationPercent = 0;
	int currentX = 0;
	int currentY = 0;
	std::mutex frameRequestMutex;
	std::deque<std::promise<std::string>> frameRequestQueue;
};


#endif /* FRAMEHANDLER_H_ */
