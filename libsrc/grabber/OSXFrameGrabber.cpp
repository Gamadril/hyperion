#include "OSXFrameGrabber.h"

#define defaultInt(dynstruct, key, value) dynstruct.contains(key) ? dynstruct[key].extract<int>() : value

OSXFrameGrabber::OSXFrameGrabber(const Poco::DynamicStruct & config, unsigned ledCount) :
	FrameGrabber(config, ledCount),
	_screenIndex(defaultInt(config, "display", 0)),
	_imgResampler(new ImageResampler())
{
	CGImageRef image;
	CGDisplayCount displayCount;
	CGDirectDisplayID displays[8];

	// get list of displays
	CGGetActiveDisplayList(8, displays, &displayCount);
	if (_screenIndex + 1 > displayCount)
	{
		std::cerr << "OSX display with index " << _screenIndex << " is not available. Using main display" << std::endl;
		_display = kCGDirectMainDisplay;
	} else {
		_display = displays[_screenIndex];
	}
		
	image = CGDisplayCreateImage(_display);
	assert(image != NULL);

	std::cout << "OSX display opened with resolution: " << CGImageGetWidth(image) << "x" << CGImageGetHeight(image) << "@" << CGImageGetBitsPerPixel(image) << "bit" << std::endl;

	CGImageRelease(image);
}

OSXFrameGrabber::~OSXFrameGrabber()
{
	delete _imgResampler;
}

void OSXFrameGrabber::grab(Image<ColorRgb> & image)
{
	CGImageRef dispImage;
	CFDataRef imgData;
	unsigned char * pImgData;	
	unsigned dspWidth, dspHeight;
	
	dispImage = CGDisplayCreateImage(_display);
	
	// dsiplay lost, use main
	if (dispImage == NULL && _display)
	{
		dispImage = CGDisplayCreateImage(kCGDirectMainDisplay);
		// no displays connected, return
		if (dispImage == NULL)
		{
			std::cerr << "OSX no display connected..." << std::endl;
			return;
		}
	}
	imgData = CGDataProviderCopyData(CGImageGetDataProvider(dispImage));
	pImgData = (unsigned char*) CFDataGetBytePtr(imgData);
	dspWidth = CGImageGetWidth(dispImage);
	dspHeight = CGImageGetHeight(dispImage);
	
	_imgResampler->setHorizontalPixelDecimation(dspWidth/_width);
	_imgResampler->setVerticalPixelDecimation(dspHeight/_height);
	_imgResampler->processImage( pImgData,
								dspWidth,
								dspHeight,
								CGImageGetBytesPerRow(dispImage),
								PIXELFORMAT_BGR32,
								image);
	
	CFRelease(imgData);
	CGImageRelease(dispImage);
}

void OSXFrameGrabber::setGrabbingMode(const GrabbingMode mode)
{
    switch (mode)
    {
        case GRABBINGMODE_VIDEO:
        case GRABBINGMODE_AUDIO:
        case GRABBINGMODE_PHOTO:
        case GRABBINGMODE_MENU:
        case GRABBINGMODE_SCREENSAVER:
            start();
            break;
        case GRABBINGMODE_OFF:
            stop();
            break;
    }
}

void OSXFrameGrabber::setVideoMode(const VideoMode mode)
{
    _imgResampler->set3D(mode);
}
