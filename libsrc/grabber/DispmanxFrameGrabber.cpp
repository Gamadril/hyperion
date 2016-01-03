#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "DispmanxFrameGrabber.h"
#include "../../include/utils/GrabbingMode.h"

#define defaultString(dynstruct, key, value) dynstruct.contains(key) ? dynstruct[key].toString() : value

DispmanxFrameGrabber::DispmanxFrameGrabber(const Poco::DynamicStruct & config, unsigned ledCount) :
	FrameGrabber(config, ledCount),
	_vc_display(0),
	_vc_resource(0),
	_vc_flags(0)
{
	// Initiase BCM
	bcm_host_init();

	{
		// Check if the display can be opened and display the current resolution
		// Open the connection to the display
		_vc_display = vc_dispmanx_display_open(0);
		assert(_vc_display > 0);

		// Obtain the display information
		DISPMANX_MODEINFO_T vc_info;
		int result = vc_dispmanx_display_get_info(_vc_display, &vc_info);
		// Keep compiler happy in 'release' mode
		(void)result;
		assert(result == 0);
		std::cout << "Display opened with resolution: " << vc_info.width << "x" << vc_info.height << std::endl;

		// Close the displaye
		vc_dispmanx_display_close(_vc_display);
	}

	// Create the resources for capturing image
	uint32_t vc_nativeImageHandle;
	_vc_resource = vc_dispmanx_resource_create(
			VC_IMAGE_RGB888,
			_width,
			_height,
			&vc_nativeImageHandle);
	assert(_vc_resource);

	// Define the capture rectangle with the same size
	vc_dispmanx_rect_set(&_rectangle, 0, 0, _width, _height);
}

DispmanxFrameGrabber::~DispmanxFrameGrabber()
{
	// Clean up resources
	vc_dispmanx_resource_delete(_vc_resource);

	// De-init BCM
	bcm_host_deinit();
}

void DispmanxFrameGrabber::grab(Image<ColorRgb> & image)
{
	// resize the given image if needed
	if (image.width() != unsigned(_rectangle.width) || image.height() != unsigned(_rectangle.height))
	{
		image.resize(_rectangle.width, _rectangle.height);
	}

	// Open the connection to the display
	_vc_display = vc_dispmanx_display_open(0);

	// Create the snapshot (incl down-scaling)
	vc_dispmanx_snapshot(_vc_display, _vc_resource, (DISPMANX_TRANSFORM_T) _vc_flags);

	// Read the snapshot into the memory
	void* image_ptr = image.memptr();
	const unsigned destPitch = _rectangle.width * sizeof(ColorRgb);
	vc_dispmanx_resource_read_data(_vc_resource, &_rectangle, image_ptr, destPitch);

	// Close the displaye
	vc_dispmanx_display_close(_vc_display);
}

void DispmanxFrameGrabber::setGrabbingMode(const GrabbingMode mode)
{
	switch (mode)
	{
	case GRABBINGMODE_VIDEO:
		_vc_flags = DISPMANX_SNAPSHOT_NO_RGB | DISPMANX_SNAPSHOT_FILL;
		start();
		break;
	case GRABBINGMODE_AUDIO:
	case GRABBINGMODE_PHOTO:
	case GRABBINGMODE_MENU:
	case GRABBINGMODE_SCREENSAVER:
		_vc_flags = 0;
		start();
		break;
	case GRABBINGMODE_OFF:
		stop();
		break;
	}
}

void DispmanxFrameGrabber::setVideoMode(const VideoMode mode)
{
	switch (mode) {
	case VIDEO_3DSBS:
		vc_dispmanx_rect_set(&_rectangle, 0, 0, _width/2, _height);
		break;
	case VIDEO_3DTAB:
		vc_dispmanx_rect_set(&_rectangle, 0, 0, _width, _height/2);
		break;
	case VIDEO_2D:
	default:
		vc_dispmanx_rect_set(&_rectangle, 0, 0, _width, _height);
		break;
	}
}
