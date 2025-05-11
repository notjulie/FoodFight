/*
 * VideoFrame.cpp
 *
 *  Created on: Oct 21, 2018
 *      Author: pi
 */

#include <iostream>
#include <sys/mman.h>
#include "VideoFrame.h"



// =====================================================
//  class VideoFrame
// =====================================================

VideoFrame::VideoFrame()
{
}

VideoFrame::~VideoFrame()
{
}

/// <summary>
/// Packages it up as a string for the benefit of text-based streams
/// </summary>
std::string VideoFrame::toString() const
{
   static char HEX_DIGITS[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	std::string result;

	int count = getPixelDataLength();
	const uint8_t *p = getPixelData();
	for (int i=0; i<count; ++i)
	{
      uint8_t b = p[i];
		result += HEX_DIGITS[(b>>4)];
		result += HEX_DIGITS[(b&0xF)];
	}
	return result;
}


// =====================================================
//  class VectorVideoFrame
// =====================================================

/// <summary>
/// Initializes a new instance of class VectorVideoFrame
/// </summary>
VectorVideoFrame::VectorVideoFrame(const uint8_t *_pixelData, size_t _pixelDataSize)
   : pixelData(_pixelData, _pixelData + _pixelDataSize)
{
}

VectorVideoFrame::~VectorVideoFrame()
{
}


// =====================================================
//  class MmapVideoFrame
// =====================================================


MmapVideoFrame::MmapVideoFrame(int fd, size_t _pixelDataSize)
   : pixelDataSize(_pixelDataSize)
{
   pixelData = (const uint8_t *)mmap(
      nullptr,
      pixelDataSize,
      PROT_READ,
      MAP_PRIVATE,
      fd,
      0
   );
}

MmapVideoFrame::~MmapVideoFrame()
{
   int result = munmap(const_cast<uint8_t *>(pixelData), pixelDataSize);
   std::cout << result;
}
