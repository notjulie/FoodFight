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

	// get the red intensity of each pixel... I want to know which pixels are red,
	// and don't have blue or green, so I just report the difference between red and
	// the larger of blue or green
	double xSum = 0;
	double ySum = 0;
	double rSum = 0;
	const uint8_t *p = frame->GetPixelData();
	auto pixelDataLength = frame->GetPixelDataLength();
	for (int i=0; i<pixelDataLength; i+=3)
	{
		int r = p[0];
		int g = p[1];
		int b = p[2];
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

	if (rSum > 0)
		printf("%d,%d\n", (int)(xSum/rSum), (int)(ySum/rSum));
}
