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


class VideoFrame {
public:
   VideoFrame();
   virtual ~VideoFrame();

	virtual int getPixelDataLength() const = 0;
	virtual const uint8_t *getPixelData() const = 0;

	std::string toString(void) const;
};


class VectorVideoFrame : public VideoFrame {
public:
	VectorVideoFrame(const uint8_t *pixelData, size_t pixelDataSize);
	virtual ~VectorVideoFrame();

	int getPixelDataLength() const override { return pixelData.size(); }
	const uint8_t *getPixelData() const override { return &pixelData[0]; }

private:
	std::vector<uint8_t> pixelData;
};


class MmapVideoFrame : public VideoFrame {
public:
	MmapVideoFrame(int fd, size_t pixelDataSize);
	virtual ~MmapVideoFrame();

	int getPixelDataLength() const override { return pixelDataSize; }
	const uint8_t *getPixelData() const override { return pixelData; }

private:
   const uint8_t *pixelData = nullptr;
   size_t pixelDataSize = 0;
};


#endif /* VIDEOFRAME_H_ */
