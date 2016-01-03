#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "FramebufferFrameGrabber.h"

#define defaultString(dynstruct, key, value) dynstruct.contains(key) ? dynstruct[key].toString() : value

FramebufferFrameGrabber::FramebufferFrameGrabber(const Poco::DynamicStruct & config, unsigned ledCount) :
	FrameGrabber(config, ledCount),
	_fbDevice(defaultString(config, "device", "/dev/fb0")),
	_imgResampler(new ImageResampler())
{
    int result;
    struct fb_var_screeninfo vinfo;

    // Check if the framebuffer device can be opened and display the current resolution
    _fbfd = open(_fbDevice.c_str(), O_RDONLY);
    if (_fbfd == -1)
    {
        std::cerr << "Error openning " << _fbDevice << std::endl;
    }
    else
    {
        // get variable screen information
        result = ioctl (_fbfd, FBIOGET_VSCREENINFO, &vinfo);
        if (result != 0)
        {
            std::cerr << "Could not get screen information" << std::endl;
        }
        else
        {
            std::cout << "Framebuffer opened with resolution: " << vinfo.xres << "x" << vinfo.yres << "@" << vinfo.bits_per_pixel << "bit" << std::endl;
            if (vinfo.bits_per_pixel == 32)
            {
                char order[4] = "";
                order[vinfo.red.offset / 8] = 'R';
                order[vinfo.green.offset / 8] = 'G';
                order[vinfo.blue.offset / 8] = 'B';
                std::cout << "Framebuffer color order: " << order << std::endl;
            }
        }
        close(_fbfd);
    }
}

FramebufferFrameGrabber::~FramebufferFrameGrabber()
{
	delete _imgResampler;
}

void FramebufferFrameGrabber::grab(Image<ColorRgb> & image)
{
    struct fb_var_screeninfo vinfo;
    unsigned capSize, bytesPerPixel;
    PixelFormat pixelFormat;

    /* Open the framebuffer device */
    _fbfd = open(_fbDevice.c_str(), O_RDONLY);

    if (_fbfd == -1)
    {
        std::cerr << "Error openning " << _fbDevice << std::endl;
        return;
    }

    /* get variable screen information */
    ioctl (_fbfd, FBIOGET_VSCREENINFO, &vinfo);

    bytesPerPixel = vinfo.bits_per_pixel / 8;
    capSize = vinfo.xres * vinfo.yres * bytesPerPixel;

    if (vinfo.bits_per_pixel == 16)
    {
        pixelFormat = PIXELFORMAT_BGR16;
    }
    else if (vinfo.bits_per_pixel == 24)
    {
        pixelFormat = PIXELFORMAT_BGR24;
    }
    else if (vinfo.bits_per_pixel == 32)
    {
        if (vinfo.red.offset == 0)
            pixelFormat = PIXELFORMAT_RGB32;
        else
            pixelFormat = PIXELFORMAT_BGR32;
    }
    else
    {
        std::cerr << "Unknown pixel format: " << vinfo.bits_per_pixel << " bits per pixel" << std::endl;
        close(_fbfd);
        return;
    }

    /* map the device to memory */
    _fbp = (unsigned char*)mmap(0, capSize, PROT_READ, MAP_PRIVATE | MAP_NORESERVE, _fbfd, 0);

    _imgResampler->setHorizontalPixelDecimation(vinfo.xres/_width);
    _imgResampler->setVerticalPixelDecimation(vinfo.yres/_height);
    _imgResampler->processImage(_fbp,
                                vinfo.xres,
                                vinfo.yres,
                                vinfo.xres * bytesPerPixel,
                                pixelFormat,
                                image);

    munmap(_fbp, capSize);
    close(_fbfd);
}

void FramebufferFrameGrabber::setGrabbingMode(const GrabbingMode mode)
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

void FramebufferFrameGrabber::setVideoMode(const VideoMode mode)
{
    _imgResampler->set3D(mode);
}
