// system includes
#include <stdexcept>
#include <cassert>
#include <iomanip>
#include <cstdio>

// stl includes
#include <iostream>
#include <sstream>
#include <iterator>

#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"

// hyperion util includes
#include "hyperion/ImageProcessorFactory.h"
#include "hyperion/ImageProcessor.h"
#include "utils/ColorRgb.h"

// project includes
#include "BoblightClientConnection.h"

BoblightClientConnection::BoblightClientConnection(Poco::Net::DialogSocket * socket, Hyperion * hyperion) :
    TCPServerConnection(*socket),
    _socket(socket),
    _imageProcessor(ImageProcessorFactory::getInstance().newImageProcessor()),
    _hyperion(hyperion),
    _priority(255),
    _ledColors(hyperion->getLedCount(), ColorRgb::BLACK)
{
}

BoblightClientConnection::~BoblightClientConnection()
{
    if (_priority < 255)
    {
        // clear the current channel
        _hyperion->clear(_priority);
    }
}

void BoblightClientConnection::run()
{
    std::string message;
    
    if (_socket->receiveMessage(message))
    {
        // handle message
        handleMessage(message);
    } else {
        std::cout << "Boblight server connection closed by peer" << std::endl;
    }
}

void BoblightClientConnection::handleMessage(const std::string & message)
{
    std::cout << "boblight message: " << message << std::endl;
    
    Poco::StringTokenizer messageParts(message, " ", Poco::StringTokenizer::TOK_IGNORE_EMPTY);

    if (messageParts.count() > 0)
    {
        if (messageParts[0] == "hello")
        {
            _socket->sendMessage("hello");
            return;
        }
        else if (messageParts[0] == "ping")
        {
            _socket->sendMessage("ping 1");
            return;
        }
        else if (messageParts[0] == "get" && messageParts.count() > 1)
        {
            if (messageParts[1] == "version")
            {
                _socket->sendMessage("version 5");
                return;
            }
            else if (messageParts[1] == "lights")
            {
                sendLightMessage();
                return;
            }
        }
        else if (messageParts[0] == "set" && messageParts.count() > 2)
        {
            if (messageParts.count() > 3 && messageParts[1] == "light")
            {
                unsigned ledIndex = Poco::NumberParser::parseUnsigned(messageParts[2]);
                if (ledIndex < _ledColors.size())
                {
                    if (messageParts[3] == "rgb" && messageParts.count() == 7)
                    {
                        uint8_t red = std::max(0, std::min(255, int(255 * Poco::NumberParser::parseFloat(messageParts[4]))));
                        uint8_t green = std::max(0, std::min(255, int(255 * Poco::NumberParser::parseFloat(messageParts[5]))));
                        uint8_t blue  = std::max(0, std::min(255, int(255 * Poco::NumberParser::parseFloat(messageParts[6]))));

                        ColorRgb & rgb =  _ledColors[ledIndex];
                        rgb.red = red;
                        rgb.green = green;
                        rgb.blue = blue;

                        // send current color values to hyperion if this is the last led assuming leds values are send in order of id
                        if ((ledIndex == _ledColors.size() -1) && _priority < 255)
                        {
                            _hyperion->setColors(_priority, _ledColors, -1);
                        }

                        return;
                    }
                    else if(messageParts[3] == "speed" ||
                            messageParts[3] == "interpolation" ||
                            messageParts[3] == "use" ||
                            messageParts[3] == "singlechange")
                    {
                        // these message are ignored by Hyperion
                        return;
                    }
                }
            }
            else if (messageParts.count() == 3 && messageParts[1] == "priority")
            {
                int prio = Poco::NumberParser::parse(messageParts[2]);
                if (prio != _priority)
                {
                    if (_priority < 255)
                    {
                        // clear the current channel
                        _hyperion->clear(_priority);
                    }

                    _priority = prio;
                    return;
                }
            }
        }
        else if (messageParts[0] == "sync")
        {
            // send current color values to hyperion
            if (_priority < 255)
            {
                _hyperion->setColors(_priority, _ledColors, -1);
            }
            return;
        }
    }

    std::cout << "unknown boblight message: " << message << std::endl;
}

void BoblightClientConnection::sendLightMessage()
{
    std::ostringstream stringStream;
    stringStream << "lights " << _hyperion->getLedCount();
    _socket->sendMessage(stringStream.str());
    
    stringStream.str(std::string());
    stringStream.clear();

    for (unsigned i = 0; i < _hyperion->getLedCount(); ++i)
    {
        double h0, h1, v0, v1;
        _imageProcessor->getScanParameters(i, h0, h1, v0, v1);
        stringStream << "lights " << i << " scan " << 100*v0 << " " << 100*v1 << " " << 100*h0 << " " << 100*h1;
        //n = snprintf(buffer, sizeof(buffer), "light %03d scan %f %f %f %f\n", i, 100*v0, 100*v1, 100*h0, 100*h1);
        _socket->sendMessage(stringStream.str());
    }
}
