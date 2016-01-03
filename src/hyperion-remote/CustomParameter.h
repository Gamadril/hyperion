#pragma once

// STL includes
#include <algorithm>

// Poco includes
#include "Poco/StringTokenizer.h"
#include "Poco/NumericString.h"

// getoptPlusPLus includes
#include <getoptPlusPlus/getoptpp.h>

// hyperion-remote includes
#include "ColorTransformValues.h"
#include "ColorValue.h"

/// Data parameter for a color
typedef vlofgren::PODParameter<std::vector<ColorValue>> ColorParameter;

/// Data parameter for color transform values (list of three values)
typedef vlofgren::PODParameter<ColorTransformValues> TransformParameter;

namespace vlofgren
{
///
/// Translates a string (as passed on the commandline) to a vector of colors
///
/// @param[in] s The string (as passed on the commandline)
///
/// @return The translated colors
///
/// @throws Parameter::ParameterRejected If the string did not result in a color
///
template<>
std::vector<ColorValue> ColorParameter::validate(const std::string& s) throw (Parameter::ParameterRejected)
{
    ColorValue color;

    // check if we can create the color by hex RRGGBB value
    if (s.length() >= 6u && (s.length()%6u) == 0u && std::count_if(s.begin(), s.end(), isxdigit) == int(s.length()))
    {
        bool ok = true;
        std::vector<ColorValue> colors;

        for (size_t j = 0; j < s.length()/6; ++j)
        {
            unsigned char rgb[3];
            for (int i = 0; i < 3 && ok; ++i)
            {
                rgb[i] = strtol(s.substr(6*j+2*i, 2).c_str(), NULL, 16);
                ok = true;
            }

            if (ok)
            {
                color.red = rgb[0];
                color.green = rgb[1];
                color.blue = rgb[2];
                colors.push_back(color);
            }
            else
            {
                break;
            }
        }

        // check if all components parsed succesfully
        if (ok)
        {
            return colors;
        }
    }

    std::stringstream errorMessage;
    errorMessage << "Invalid color. A color is specified by a six lettered RRGGBB hex value.";
    throw Parameter::ParameterRejected(errorMessage.str());

    return std::vector<ColorValue> {color};
}

template<>
ColorTransformValues TransformParameter::validate(const std::string& s) throw (Parameter::ParameterRejected)
{
    ColorTransformValues transform;

    // s should be split in 3 parts
    // seperators are either a ',' or a space
    Poco::StringTokenizer components(s, " ", Poco::StringTokenizer::TOK_IGNORE_EMPTY);

    if (components.count() == 3)
    {
        bool ok1, ok2, ok3;
        ok1 = Poco::strToDouble(components[0], transform.valueRed);
        ok2 = Poco::strToDouble(components[1], transform.valueGreen);
        ok3 = Poco::strToDouble(components[2], transform.valueBlue);

        if (ok1 && ok2 && ok3)
        {
            return transform;
        }
    }

    std::stringstream errorMessage;
    errorMessage << "Argument " << s << " can not be parsed to 3 double values";
    throw Parameter::ParameterRejected(errorMessage.str());

    return transform;
}
}
