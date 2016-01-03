#pragma once

#include <CoreGraphics/CoreGraphics.h>

#include "grabber/FrameGrabber.h"
#include "utils/ImageResampler.h"

///
/// The OSXFrameGrabber captures the content of the display into a ImageRgb.
/// This ImageRgb is processed to a ColorRgb for each led and commmited to the attached Hyperion.
///
class OSXFrameGrabber: public FrameGrabber
{
public:
    ///
    /// Constructs the OSX display frame grabber based on the specified config
    ///
    /// @param[in] config Framebuffer config
    ///
    OSXFrameGrabber(const Poco::DynamicStruct & config, unsigned ledCount);

    ///
    /// Destructor of this frame grabber. Releases any claimed resources.
    ///
    ~OSXFrameGrabber();
	
    void setGrabbingMode(const GrabbingMode mode);

    void setVideoMode(const VideoMode mode);
	
protected:
	void grab(Image<ColorRgb> & image);

private:
	/// display
	const unsigned _screenIndex;
		
	/// Reference to the captured diaplay
	CGDirectDisplayID _display;
	
    /// Image resampler for downscaling the image
    ImageResampler * _imgResampler;
};
