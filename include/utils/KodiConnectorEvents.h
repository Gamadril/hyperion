#pragma once

#include "Poco/BasicEvent.h"

#include "VideoMode.h"
#include "GrabbingMode.h"

class KodiConnectorEvents
{
public:
    Poco::BasicEvent<const VideoMode> videoModeEvent;
    Poco::BasicEvent<const GrabbingMode> grabbingModeEvent;
};