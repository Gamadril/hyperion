
#pragma once

// Poco includes
#include "Poco/Dynamic/Struct.h"

// Leddevice includes
#include "LedDevice.h"

///
/// The LedDeviceFactory is responsible for constructing 'LedDevices'
///
class LedDeviceFactory
{
public:

    ///
    /// Constructs a LedDevice based on the given configuration
    ///
    /// @param deviceConfig The configuration of the led-device
    ///
    /// @return The constructed LedDevice or nullptr if configuration is invalid. The ownership of
    /// the constructed LedDevice is tranferred to the caller
    ///
    static LedDevice * construct(const Poco::DynamicStruct & deviceConfig);
};
