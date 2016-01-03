#pragma once

#include <vector>

#include "Poco/Timer.h"
#include "Poco/Dynamic/Struct.h"

#include "Poco/BasicEvent.h"

#include "hyperion/Hyperion.h"
#include "utils/HyperionControlEvents.h"
#include "utils/GrabbingMode.h"
#include "utils/VideoMode.h"
#include "utils/ColorRgb.h"
#include "hyperion/ImageProcessor.h"

///
/// Abstract class for FrameGrabber.
///
class FrameGrabber: public HyperionControlEvents
{
public:
	FrameGrabber(const Poco::DynamicStruct & config, unsigned ledCount);
    virtual ~FrameGrabber();
	
    ///
    /// Starts the grabber wich produces led values with the specified update rate
    ///
    void start();

    ///
    /// Stops the grabber
    ///
    void stop();

    ///
    /// Set the grabbing mode
    /// @param[in] mode The new grabbing mode
    ///
    virtual void setGrabbingMode(const GrabbingMode mode) = 0;

    ///
    /// Set the video mode (2D/3D)
    /// @param[in] mode The new video mode
    ///
    virtual void setVideoMode(const VideoMode mode) = 0;

protected:
	virtual void grab(Image<ColorRgb> & image) = 0;
    /// With of the captured snapshot [pixels]
    const unsigned _width;
    /// Height of the captured snapshot [pixels]
    const unsigned _height;	
	
private:
    /// The image used for grabbing frames
    Image<ColorRgb> _image;	
    /// The priority of the led colors
    const int _priority;
    /// The list with computed led colors
    std::vector<ColorRgb> _ledColors;
	/// Timer object
	Poco::Timer _timer;
    /// The processor for transforming images to led colors
    ImageProcessor * _processor;

	void onTimer(Poco::Timer& timer);
    bool _timerRunning;
};
