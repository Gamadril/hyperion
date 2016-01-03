#pragma once

//#pragma GCC system_header
#include <bcm_host.h>

#include "grabber/FrameGrabber.h"

///
/// The FramebufferFrameGrabber captures the image from the framebuffer device into a ImageRgb.
/// This ImageRgb is processed to a ColorRgb for each led and commmited to the attached Hyperion.
///
class DispmanxFrameGrabber: public FrameGrabber
{
public:
    ///
    /// Constructs the framebuffer frame grabber based on the specified config
    ///
    /// @param[in] config Framebuffer config
    ///
    DispmanxFrameGrabber(const Poco::DynamicStruct & config, unsigned ledCount);

    ///
    /// Destructor of this framebuffer frame grabber. Releases any claimed resources.
    ///
    ~DispmanxFrameGrabber();
	
    void setGrabbingMode(const GrabbingMode mode);

    void setVideoMode(const VideoMode videoMode);
	
protected:
	void grab(Image<ColorRgb> & image);

private:
	/// Handle to the display that is being captured
	DISPMANX_DISPLAY_HANDLE_T _vc_display;

	/// Handle to the resource for storing the captured snapshot
	DISPMANX_RESOURCE_HANDLE_T _vc_resource;

	/// Rectangle of the captured resource that is transfered to user space
	VC_RECT_T _rectangle;

	/// Flags (transforms) for creating snapshots
	int _vc_flags;
};
