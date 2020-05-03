/*
 * VideoFrame.h
 *
 *  Created on: Oct 21, 2018
 *      Author: pi
 */

#ifndef VIDEOFRAME_H_
#define VIDEOFRAME_H_

#include <string>
#include <vector>
#include "interface/mmal/mmal.h"


class VideoFrame {
public:
	VideoFrame(MMAL_BUFFER_HEADER_T *buffer);

	int GetPixelDataLength(void) const { return pixelData.size(); }
	const uint8_t *GetPixelData(void) const { return &pixelData[0]; }

	std::string ToString(void) const;

private:
	std::vector<uint8_t> pixelData;
};


#endif /* VIDEOFRAME_H_ */
