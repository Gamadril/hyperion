#pragma once

#include "grabber/FrameGrabber.h"
#include "utils/ImageResampler.h"

#include <X11/Xlib.h>

///
/// The X11Grabber captures the content of the display into a ImageRgb.
/// This ImageRgb is processed to a ColorRgb for each led and commmited to the attached Hyperion.
///
class X11FrameGrabber : public FrameGrabber
{
public:
    ///
    /// Constructs the X11 frame grabber based on the specified config
    ///
    /// @param[in] config Framebuffer config
    ///
    X11FrameGrabber(const Poco::DynamicStruct & config, unsigned ledCount);

    ///
    /// Destructor of this frame grabber. Releases any claimed resources.
    ///
    ~X11FrameGrabber();
	
    void setGrabbingMode(const GrabbingMode mode);

    void setVideoMode(const VideoMode mode);
	
protected:
	void grab(Image<ColorRgb> & image);

private:
    /// Image resampler for downscaling the image
    ImageResampler * _imgResampler;
    Display *_x11Display;
};
