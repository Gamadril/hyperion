#include "LedDeviceAmbiLed.h"

LedDeviceAmbiLed::LedDeviceAmbiLed(const std::string& outputDevice, const unsigned baudrate, int delayAfterConnect_ms) :
    LedRs232Device(outputDevice, baudrate, delayAfterConnect_ms),
    _ledBuffer(0)
{
}

int LedDeviceAmbiLed::write(const std::vector<ColorRgb> & ledValues)
{
    if (_ledBuffer.size() == 0)
    {
        _ledBuffer.resize(1 + 3*ledValues.size());
        _ledBuffer[3*ledValues.size()] = 255;
    }

    // write data
    memcpy( _ledBuffer.data(), ledValues.data(), ledValues.size() * 3);
    return writeBytes(_ledBuffer.size(), _ledBuffer.data());
}

int LedDeviceAmbiLed::switchOff()
{
    // write data
    memset(_ledBuffer.data(), 0, _ledBuffer.size()-6);
    return writeBytes(_ledBuffer.size(), _ledBuffer.data());
}
