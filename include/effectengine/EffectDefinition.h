#pragma once

#include "Poco/Dynamic/Struct.h"

#include <string>

struct EffectDefinition
{
    std::string name;
    std::string script;
    Poco::DynamicStruct args;
};
