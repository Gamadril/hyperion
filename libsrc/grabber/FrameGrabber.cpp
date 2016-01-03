#include "grabber/FrameGrabber.h"

FrameGrabber::FrameGrabber(const Poco::DynamicStruct & config, unsigned ledCount):
	_width((unsigned) config["width"].extract<int>()),
    _height((unsigned) config["height"].extract<int>()),
	_image(_width, _height),
	_priority(1000),
    _ledColors(ledCount, ColorRgb {0,0,0}),
	_timer(0, (long) config["period"].extract<int>()),
    _processor(ImageProcessorFactory::getInstance().newImageProcessor()),
    _timerRunning(false)
{
	_processor->setSize(_width, _height);
}

FrameGrabber::~FrameGrabber()
{
    delete _processor;
}

void FrameGrabber::start() {
    if (!_timerRunning) {
        Poco::TimerCallback<FrameGrabber> timerCallback(*this, &FrameGrabber::onTimer);
        _timer.start(timerCallback);
        _timerRunning = true;
    }
}

void FrameGrabber::stop() {
    if (_timerRunning) {
        _timer.stop();
        clearEvent.notifyAsync(this, _priority);
        _timerRunning = false;
    }
}

void FrameGrabber::onTimer(Poco::Timer& timer)
{
	grab(_image);
	_processor->process(_image, _ledColors);
    ColorSetArgs args = {_priority, _ledColors, 0, true};
    setColorsEvent.notifyAsync(this, args);
}
