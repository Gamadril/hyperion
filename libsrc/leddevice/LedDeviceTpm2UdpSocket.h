#pragma once

// STL includes
#include <string>

// hyperion incluse
#include "UdpSocketDevice.h"

///
/// Implementation of the LedDevice interface for writing to udp socket device using tpm2 protocol.
///
class LedDeviceTpm2UdpSocket : public UdpSocketDevice
{
public:
    ///
    /// Constructs the LedDevice for network device supporting tpm2 protocol
    /// All LEDs in the stripe are handled as one frame
    ///
    /// @param address the address of the device includeing the port
    ///
    LedDeviceTpm2UdpSocket(const std::string& address);

    ///
    /// Writes the led color values to the led-device
    ///
    /// @param ledValues The color-value per led
    /// @return Zero on succes else negative
    ///
    virtual int write(const std::vector<ColorRgb> &ledValues);

    /// Switch the leds off
    virtual int switchOff();

private:
    /// The buffer containing the packed RGB values
    std::vector<uint8_t> _ledBuffer;
};
