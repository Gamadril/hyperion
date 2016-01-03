#include "grabber/FrameGrabberFactory.h"

#include "HyperionConfig.h"

#ifdef ENABLE_GRABBER_FB
#include "FramebufferFrameGrabber.h"
#endif
#ifdef ENABLE_GRABBER_OSX
#include "OSXFrameGrabber.h"
#endif
#ifdef ENABLE_GRABBER_DISPMANX
#include "DispmanxFrameGrabber.h"
#endif
#ifdef ENABLE_GRABBER_V42L
#include "V4L2FrameGrabber.h"
#endif
#ifdef ENABLE_GRABBER_X11
#include "X11FrameGrabber.h"
#endif

FrameGrabber * FrameGrabberFactory::createNewFrameGrabber(const Poco::DynamicStruct & config, unsigned ledCount)
{
    std::string type = config["type"].toString();
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);

    FrameGrabber* frameGrabber = nullptr;
	
	if (type == "test")
	{
		
	}
#ifdef ENABLE_GRABBER_FB
	else if (type == "fb")
	{
		frameGrabber = new FramebufferFrameGrabber(config, ledCount);
	}
#endif
#ifdef ENABLE_GRABBER_DISPMANX
	else if (type == "dispmanx")
	{
		frameGrabber = new DispmanxFrameGrabber(config, ledCount);
	}
#endif
#ifdef ENABLE_GRABBER_OSX
	else if (type == "osx")
	{
		frameGrabber = new OSXFrameGrabber(config, ledCount);
	}
#endif
#ifdef ENABLE_GRABBER_V42L
	else if (type == "v42l")
	{
		frameGrabber = new V4L2FrameGrabber(config, hyperion);
	}
#endif
#ifdef ENABLE_GRABBER_X11
	else if (type == "x11")
	{
		frameGrabber = new X11FrameGrabber(config, ledCount);
	}
#endif
	
    return frameGrabber;
}
