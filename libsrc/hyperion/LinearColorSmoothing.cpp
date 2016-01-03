#include <string.h>

#include "Poco/Timestamp.h"

#include "LinearColorSmoothing.h"

LinearColorSmoothing::LinearColorSmoothing(
    LedDevice * ledDevice,
    double ledUpdateFrequency_hz,
    int settlingTime_ms,
    unsigned updateDelay) :
    LedDevice(),
    _ledDevice(ledDevice),
    _updateInterval(1000 / ledUpdateFrequency_hz),
    _settlingTime(settlingTime_ms),
    _timer(0, _updateInterval),
    _outputDelay(updateDelay)
{
    std::cout << "Created linear-smoothing(interval_ms=" << _updateInterval << ";settlingTime_ms=" << settlingTime_ms << ";updateDelay=" << _outputDelay << std::endl;
}

LinearColorSmoothing::~LinearColorSmoothing()
{
    // Make sure to switch off the underlying led-device (because switchOff is no longer forwarded)
    _ledDevice->switchOff();
    delete _ledDevice;
}

void LinearColorSmoothing::onTimer(Poco::Timer& timer)
{
	updateLeds();
}

int LinearColorSmoothing::write(const std::vector<ColorRgb> &ledValues)
{
	Poco::Timestamp now;
    // received a new target color
    if (_previousValues.empty())
    {
        // not initialized yet
        _targetTime = (now.epochMicroseconds() / 1000) + _settlingTime;
        _targetValues = ledValues;

        _previousTime = (now.epochMicroseconds() / 1000);
        _previousValues = ledValues;
		 Poco::TimerCallback<LinearColorSmoothing> tc(*this, &LinearColorSmoothing::onTimer);
		_timer.start(tc);
    }
    else
    {
        _targetTime = (now.epochMicroseconds() / 1000) + _settlingTime;
        memcpy(_targetValues.data(), ledValues.data(), ledValues.size() * sizeof(ColorRgb));
    }

    return 0;
}

int LinearColorSmoothing::switchOff()
{
    // We will keep updating the leds (but with pure-black)

    // Clear the smoothing parameters
    std::fill(_targetValues.begin(), _targetValues.end(), ColorRgb::BLACK);
    _targetTime = 0;

    // Erase the output-queue
    for (unsigned i=0; i<_outputQueue.size(); ++i)
    {
        _outputQueue.push_back(_targetValues);
        _outputQueue.pop_front();
    }

    return 0;
}

void LinearColorSmoothing::updateLeds()
{
	Poco::Timestamp timestamp;
    int64_t now = (timestamp.epochMicroseconds() / 1000);
    int deltaTime = _targetTime - now;

    if (deltaTime < 0)
    {
        memcpy(_previousValues.data(), _targetValues.data(), _targetValues.size() * sizeof(ColorRgb));
        _previousTime = now;

        queueColors(_previousValues);
    }
    else
    {
        float k = 1.0f - 1.0f * deltaTime / (_targetTime - _previousTime);

        for (size_t i = 0; i < _previousValues.size(); ++i)
        {
            ColorRgb & prev = _previousValues[i];
            ColorRgb & target = _targetValues[i];

            prev.red   += k * (target.red   - prev.red);
            prev.green += k * (target.green - prev.green);
            prev.blue  += k * (target.blue  - prev.blue);
        }
        _previousTime = now;

        queueColors(_previousValues);
    }
}

void LinearColorSmoothing::queueColors(const std::vector<ColorRgb> & ledColors)
{
    if (_outputDelay == 0)
    {
        // No output delay => immediate write
        _ledDevice->write(ledColors);
    }
    else
    {
        // Push the new colors in the delay-buffer
        _outputQueue.push_back(ledColors);
        // If the delay-buffer is filled pop the front and write to device
        if (_outputQueue.size() > _outputDelay)
        {
            _ledDevice->write(_outputQueue.front());
            _outputQueue.pop_front();
        }
    }
}
