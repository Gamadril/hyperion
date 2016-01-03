#pragma once

#include "Poco/BasicEvent.h"

#include "DelegateArgs.h"

class HyperionControlEvents
{
public:
    Poco::BasicEvent<ColorSetArgs> setColorsEvent;
    Poco::BasicEvent<const int> clearEvent;
    Poco::BasicEvent<void> clearAllEvent;
};
