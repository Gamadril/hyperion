// STL includes
#include <cstring>
#include <unistd.h>
#include <iostream>

// hyperion includes
#include <hyperion/LedString.h>


LedString::LedString()
{
    // empty
}

LedString::~LedString()
{
    // empty
}

unsigned LedString::count() const
{
    return (unsigned) _leds.size();
}

std::vector<Led>& LedString::leds()
{
    return _leds;
}

const std::vector<Led>& LedString::leds() const
{
    return _leds;
}
