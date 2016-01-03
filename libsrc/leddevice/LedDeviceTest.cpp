#include <fstream>

// Local-Hyperion includes
#include "LedDeviceTest.h"

LedDeviceTest::LedDeviceTest(const std::string& output) :
    _fileName(output.c_str())
{
    // empty
}

LedDeviceTest::~LedDeviceTest()
{
    // empty
}

int LedDeviceTest::write(const std::vector<ColorRgb> & ledValues)
{
	std::ofstream file(_fileName);
    for (const ColorRgb& color : ledValues)
    {
        file << color;
    }
    file << std::endl;
	file.close();

    return 0;
}

int LedDeviceTest::switchOff()
{
    return 0;
}
