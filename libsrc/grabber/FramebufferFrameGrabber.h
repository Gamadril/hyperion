#pragma once

#include "grabber/FrameGrabber.h"
#include "utils/ImageResampler.h"

///
/// The FramebufferFrameGrabber captures the image from the framebuffer device into a ImageRgb.
/// This ImageRgb is processed to a ColorRgb for each led and commmited to the attached Hyperion.
///
class FramebufferFrameGrabber: public FrameGrabber
{
public:
    ///
    /// Constructs the framebuffer frame grabber based on the specified config
    ///
    /// @param[in] config Framebuffer config
    ///
    FramebufferFrameGrabber(const Poco::DynamicStruct & config, unsigned ledCount);

    ///
    /// Destructor of this framebuffer frame grabber. Releases any claimed resources.
    ///
    ~FramebufferFrameGrabber();
	
    void setGrabbingMode(const GrabbingMode mode);

    void setVideoMode(const VideoMode videoMode);
	
protected:
	void grab(Image<ColorRgb> & image);

private:
    /// Framebuffer file descriptor
    int _fbfd;
    /// Pointer to framebuffer
    unsigned char * _fbp;
    /// Framebuffer device e.g. /dev/fb0
    const std::string _fbDevice;
    /// Image resampler for downscaling the image
    ImageResampler * _imgResampler;
};
