#pragma once

// Poco includes
#include "Poco/Dynamic/Struct.h"

#include "FrameGrabber.h"

class Hyperion;

///
/// The FrameGrabberFactory is responsible for constructing frame grabbers
///
class FrameGrabberFactory
{
public:

    ///
    /// Creates a FrameGrabber based on the given configuration
    ///
    /// @param config The configuration of the grabber
    ///
    /// @return The constructed FrameGrabber or nullptr if configuration is invalid. The ownership of
    /// the constructed FrameGrabber is tranferred to the caller
    ///
    static FrameGrabber * createNewFrameGrabber(const Poco::DynamicStruct & config, unsigned ledCount);
};
