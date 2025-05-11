/*
 * FrameHandler.cpp
 *
 *  Created on: Oct 21, 2018
 *      Author: pi
 */

#include <algorithm>
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

   int saturatedCount = 0;

	// get the red intensity of each pixel... I want to know which pixels are red,
	// and don't have blue or green, so I just report the difference between red and
	// the larger of blue or green
	double xSum = 0;
	double ySum = 0;
	double rSum = 0;
	const uint8_t *p = frame->getPixelData();
	auto pixelDataLength = frame->getPixelDataLength();
	for (int i=0; i<pixelDataLength; i+=3)
	{
		int r = p[0];
		int g = p[1];
		int b = p[2];
		if (r == 255)
         ++saturatedCount;
		if (g == 255)
         ++saturatedCount;
		if (b == 255)
         ++saturatedCount;

		int gray = std::min(g, b);
		r -= gray;
		g -= gray;
		b -= gray;

		int x = (i/3)%640;
		int y = (i/3)/640;

		if (r >= 3*std::max(g, b))
		{
			r = r - 3*std::max(g, b);
			double weight = r * r;
			xSum += weight*x;
			ySum += weight*y;
			rSum += weight;
		}
		p += 3;
	}

	// note the saturation rate
	this->saturationPercent = 100.0 * saturatedCount / frame->getPixelDataLength() / 3;

	// process any requests for frames from TCP clients
	{
		std::lock_guard<std::mutex> lock(frameRequestMutex);
		while (!frameRequestQueue.empty())
		{
			frameRequestQueue.front().set_value(frame->toString());
			frameRequestQueue.pop_front();
		}
	}
}


/// <summary>
/// Returns an image as a string, so that we can report it over out TCP socket.
/// This makes a request to whatever thread the camera runs on and waits on the
/// request.
/// </summary>
std::string FrameHandler::GetImageAsString(void)
{
	// create the request
	std::promise<std::string> frameRequest;

	// get the associated future that will return the result
	std::future<std::string> future = frameRequest.get_future();

	// pop it in the queue
	{
		std::lock_guard<std::mutex> lock(frameRequestMutex);
		frameRequestQueue.push_back(std::move(frameRequest));
	}

	// wait and return the result
	return future.get();
}



