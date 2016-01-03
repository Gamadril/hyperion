// STL includes
//#include <iostream>
//#include <cstdint>

// X11 includes
#include <X11/Xutil.h>

/*
X11Grabber::X11Grabber(int cropLeft, int cropRight, int cropTop, int cropBottom, int horizontalPixelDecimation, int verticalPixelDecimation) :
    _imageResampler(),
    _cropLeft(cropLeft),
    _cropRight(cropRight),
    _cropTop(cropTop),
    _cropBottom(cropBottom),
    _x11Display(nullptr),
    _screenWidth(0),
    _screenHeight(0),
    _image(0,0)
{
    _imageResampler.setHorizontalPixelDecimation(horizontalPixelDecimation);
    _imageResampler.setVerticalPixelDecimation(verticalPixelDecimation);
    _imageResampler.setCropping(0, 0, 0, 0); // cropping is performed by XGetImage
}

*/


#include "X11FrameGrabber.h"
#include "../../include/utils/PixelFormat.h"

X11FrameGrabber::X11FrameGrabber(const Poco::DynamicStruct & config, unsigned ledCount) :
        FrameGrabber(config, ledCount),
        _imgResampler(new ImageResampler()),
        _x11Display(nullptr)
{
    _x11Display = XOpenDisplay(NULL);

    if (_x11Display == nullptr)
    {
        std::cerr << "Failed to open the default X11-display" << std::endl;
    } else {
        XWindowAttributes window_attributes_return;
        const Status status = XGetWindowAttributes(_x11Display, DefaultRootWindow(_x11Display), &window_attributes_return);
        if (status == 0)
        {
            std::cerr << "Failed to obtain window attributes" << std::endl;
        } else {
            std::cout << "X11 dsiplay opened with resolution: " << window_attributes_return.width << "x" << window_attributes_return.height << "@" << window_attributes_return.depth << "bit" << std::endl;
        }
    }
}

X11FrameGrabber::~X11FrameGrabber()
{
    if (_x11Display != nullptr)
    {
        XCloseDisplay(_x11Display);
    }
}

void X11FrameGrabber::grab(Image<ColorRgb> & image)
{
    PixelFormat pixelFormat;

    // Capture the current screen
    XImage * xImage = XGetImage(_x11Display, DefaultRootWindow(_x11Display), 0, 0, XDisplayWidth(_x11Display,0), XDisplayHeight(_x11Display,0), AllPlanes, ZPixmap);
    if (xImage == nullptr)
    {
        std::cerr << "Grab failed" << std::endl;
        return;
    }

    if (xImage->depth == 16)
    {
        pixelFormat = PIXELFORMAT_BGR16;
    }
    else if (xImage->depth == 24) {
        if ((xImage->bytes_per_line / xImage->width) == 3) {
            pixelFormat = PIXELFORMAT_BGR24;
        } else {
            pixelFormat = PIXELFORMAT_BGR32;
        }
    }
    else if (xImage->depth == 32) {
        pixelFormat = PIXELFORMAT_BGR32;
    }
    else {
        std::cerr << "Unknown color depth: " << xImage->depth << " with " << (xImage->bytes_per_line / xImage->width) <<
        " bytes per pixel" << std::endl;
        XDestroyImage(xImage);
        return;
    }

    _imgResampler->setHorizontalPixelDecimation(xImage->width/_width);
    _imgResampler->setVerticalPixelDecimation(xImage->height/_height);
    _imgResampler->processImage((const uint8_t *)xImage->data,
                                xImage->width,
                                xImage->height,
                                xImage->bytes_per_line,
                                pixelFormat,
                                image);

    // Cleanup allocated resources of the X11 grab
    XDestroyImage(xImage);
}

void X11FrameGrabber::setGrabbingMode(const GrabbingMode mode)
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

void X11FrameGrabber::setVideoMode(const VideoMode mode)
{
    _imgResampler->set3D(mode);
}
