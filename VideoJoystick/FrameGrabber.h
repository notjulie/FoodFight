
#ifndef FRAMEGRABBER_H
#define FRAMEGRABBER_H

#include <functional>
#include <memory>
#include "VideoFrame.h"

/// <summary>
/// Abstract representation of a frame grabber
/// </summary>
class FrameGrabber
{
public:
	FrameGrabber() {}
	virtual ~FrameGrabber() {}

	virtual void CreateCameraComponent() {}
	virtual void SetupFrameCallback(const std::function<void(const std::shared_ptr<VideoFrame> &)> &callback) {}
	virtual void StartCapturing() {}
};


#endif
