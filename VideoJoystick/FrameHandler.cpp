/*
 * FrameHandler.cpp
 *
 *  Created on: Oct 21, 2018
 *      Author: pi
 */

#include <algorithm>
#include "FrameHandler.h"


/// <summary>
/// Initializes a new instance of class FrameHandler
/// </summary>
FrameHandler::FrameHandler()
{
}


/// <summary>
/// Processes the frame, locates the dot, calls the callback
/// </summary>
void FrameHandler::HandleFrame(const std::shared_ptr<VideoFrame> &frame)
{
   auto start = std::chrono::steady_clock::now();

	// skip the first several frames until the camera warms up
	++framesReceived;
	if (framesReceived < 100)
		return;

   int saturatedCount = 0;

   std::vector<int> xValues;
   std::vector<int> yValues;

   // To process the entire 640x480 image takes about 100ms; this is
   // a prime number that limits the number of pixels we actually sample.
   // Choosing a prime number is close enough to a random sampling.  A value
   // of 307 drops processing time down to about 0.4ms, not counting the
   // callback.
   constexpr int scaledown = 307;

	// get the red intensity of each pixel... I want to know which pixels are red,
	// and don't have blue or green, so I just report the difference between red and
	// the larger of blue or green
	const uint8_t *p = frame->getPixelData();
	auto pixelDataLength = frame->getPixelDataLength();
	int i = 0;
	for (int n=0; n<10000; ++n)
	{
      // our current camera setup returns 24-bit BGR
		uint8_t b = p[i];
		uint8_t g = p[i+1];
      uint8_t r = p[i+2];
		if (r == 255)
         ++saturatedCount;
		if (g == 255)
         ++saturatedCount;
		if (b == 255)
         ++saturatedCount;

      // really simple... we are looking for red pixels; anything where
      // r > b + g is a really quality criterion
      if (r > b + g)
      {
         int x = (i/3)%640;
         int y = (i/3)/640;
         xValues.push_back(x);
         yValues.push_back(y);
      }

      i += 3*scaledown;
      if (i >= pixelDataLength)
         i -= pixelDataLength;
	}

	// just to reduce our worries about wild data points we use the
	// median as our result
	if (xValues.size() > 0)
	{
      std::sort(xValues.begin(), xValues.end());
      std::sort(yValues.begin(), yValues.end());
      this->currentX = xValues[xValues.size() / 2];
      this->currentY = yValues[yValues.size() / 2];
      if (frameCallback)
         frameCallback(this->currentX, this->currentY);
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

	auto elapsed = std::chrono::steady_clock::now() - start;
	frameProcessTime = std::chrono::duration_cast<std::chrono::microseconds>(elapsed);
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



