// stl includes
#include <stdexcept>
#include <sstream>

#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"
#include "Poco/Format.h"
#include "Poco/JSON/Array.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/Net/SocketAddress.h"

#include <png.h>

// hyperion-remote includes
#include "JsonConnection.h"

JsonConnection::JsonConnection(const std::string & address, bool printJson) :
    _printJson(printJson),
    _socket()
{
    Poco::StringTokenizer tokenizer(address, ":", Poco::StringTokenizer::TOK_TRIM);

    if (tokenizer.count() != 2)
    {
        std::string error = Poco::format("Wrong address: unable to parse address (%s)", address);
        throw std::runtime_error(error);
    }

    unsigned port;
    bool ok = Poco::NumberParser::tryParseUnsigned(tokenizer[1], port);
    if (!ok)
    {
        throw std::runtime_error(Poco::format("Wrong address: Unable to parse the port number (%s)", tokenizer[1]));
    }

    _socket.connect(Poco::Net::SocketAddress(tokenizer[0], port));
    std::cout << "Connected to " << address << std::endl;
}

JsonConnection::~JsonConnection()
{
    _socket.close();
}

void JsonConnection::setColor(std::vector<ColorValue> colors, int priority, int duration)
{
    std::cout << "Set color to " << static_cast<unsigned>(colors[0].red) << " " << static_cast<unsigned>(colors[0].green) << " " << static_cast<unsigned>(colors[0].blue) << (colors.size() > 1 ? " + ..." : "") << std::endl;

    // create command
    Poco::JSON::Object command;
    command.set("command", "color");
    command.set("priority", priority);

    Poco::JSON::Array clr;
    for (const ColorValue & color : colors)
    {
        clr.add(color.red);
        clr.add(color.green);
        clr.add(color.blue);
    }
    command.set("color", clr);
    if (duration > 0)
    {
        command.set("duration", duration);
    }

    // send command message
    Poco::Dynamic::Var reply = sendMessage(command);

    // parse reply message
    parseReply(reply);
}

void JsonConnection::setImage(std::string imagepath, int priority, int duration)
{
    std::cout << "Set image: " << imagepath << std::endl;
    //std::cout << "Set image has size: " << image.width() << "x" << image.height() << std::endl;

    // ensure the image has RGB888 format
    /*
    image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QByteArray binaryImage;
    binaryImage.reserve(image.width() * image.height() * 3);
    for (int i = 0; i < image.height(); ++i)
    {
    	const QRgb * scanline = reinterpret_cast<const QRgb *>(image.scanLine(i));
    	for (int j = 0; j < image.width(); ++j)
    	{
    		binaryImage.append((char) qRed(scanline[j]));
    		binaryImage.append((char) qGreen(scanline[j]));
    		binaryImage.append((char) qBlue(scanline[j]));
    	}
    }
    const QByteArray base64Image = binaryImage.toBase64();
    */

    // create command
    Poco::JSON::Object command;
    command.set("command", "image");
    command.set("priority", priority);
    //command.set("imagewidth", image.width());
    //command.set("imageheight", image.height());
    //command["imagedata"] = std::string(base64Image.data(), base64Image.size());
    if (duration > 0)
    {
        command.set("duration", duration);
    }

    // send command message
    //Json::Value reply = sendMessage(command);

    // parse reply message
    //parseReply(reply);
}

void JsonConnection::setEffect(const std::string & effectName, const std::string & effectArgs, int priority, int duration)
{
    std::cout << "Start effect " << effectName << std::endl;

    // create command
    Poco::JSON::Object command;
    command.set("command", "effect");
    command.set("priority", priority);

    Poco::JSON::Object effect;
    effect.set("name", effectName);
    if (effectArgs.size() > 0)
    {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var args;
        try
        {
            args = parser.parse(effectArgs);
        }
        catch(Poco::Exception& ex)
        {
            throw std::runtime_error("Error in effect arguments: " + ex.message());
        }

        effect.set("args", args);
    }
    command.set("effect", effect);

    if (duration > 0)
    {
        command.set("duration", duration);
    }

    // send command message
    Poco::Dynamic::Var reply = sendMessage(command);

    // parse reply message
    parseReply(reply);
}

std::string JsonConnection::getServerInfo()
{
    std::cout << "Get server info" << std::endl;

    // create command
    Poco::JSON::Object command;
    command.set("command", "serverinfo");

    // send command message
    Poco::Dynamic::Var reply = sendMessage(command);

    // parse reply message
    if (parseReply(reply))
    {
        Poco::JSON::Object::Ptr object = reply.extract<Poco::JSON::Object::Ptr>();
        std::string ss = object->get("info").toString();

        return ss;
    }

    return "";
}

void JsonConnection::clear(int priority)
{
    std::cout << "Clear priority channel " << priority << std::endl;

    // create command
    Poco::JSON::Object command;
    command.set("command", "clear");
    command.set("priority", priority);

    // send command message
    Poco::Dynamic::Var reply = sendMessage(command);

    // parse reply message
    parseReply(reply);
}

void JsonConnection::clearAll()
{
    std::cout << "Clear all priority channels" << std::endl;

    // create command
    Poco::JSON::Object command;
    command.set("command", "clearall");

    // send command message
    Poco::Dynamic::Var reply = sendMessage(command);

    // parse reply message
    parseReply(reply);
}

void JsonConnection::setTransform(std::string * transformId, double * saturation, double * value, ColorTransformValues *threshold, ColorTransformValues *gamma, ColorTransformValues *blacklevel, ColorTransformValues *whitelevel)
{
    std::cout << "Set color transforms" << std::endl;

    // create command
    Poco::JSON::Object command;
    command.set("command", "transform");

    Poco::JSON::Object transform;
    if (transformId != nullptr)
    {
        transform.set("id", *transformId);
    }

    if (saturation != nullptr)
    {
        transform.set("saturationGain", *saturation);
    }

    if (value != nullptr)
    {
        transform.set("valueGain", *value);
    }


    if (threshold != nullptr)
    {
        Poco::JSON::Array thrs;
        thrs.add(threshold->valueRed);
        thrs.add(threshold->valueGreen);
        thrs.add(threshold->valueBlue);
        transform.set("threshold", thrs);
    }


    if (gamma != nullptr)
    {
        Poco::JSON::Array gma;
        gma.add(gamma->valueRed);
        gma.add(gamma->valueGreen);
        gma.add(gamma->valueBlue);
        transform.set("gamma", gma);
    }

    if (blacklevel != nullptr)
    {
        Poco::JSON::Array bl;
        bl.add(gamma->valueRed);
        bl.add(gamma->valueGreen);
        bl.add(gamma->valueBlue);
        transform.set("blacklevel", bl);
    }

    if (whitelevel != nullptr)
    {
        Poco::JSON::Array wl;
        wl.add(gamma->valueRed);
        wl.add(gamma->valueGreen);
        wl.add(gamma->valueBlue);
        transform.set("whitelevel", wl);
    }
    command.set("transform", transform);

    // send command message
    Poco::Dynamic::Var reply = sendMessage(command);

    // parse reply message
    parseReply(reply);
}

Poco::Dynamic::Var JsonConnection::sendMessage(const Poco::JSON::Object & message)
{
    std::string response;

    // serialize message
    std::ostringstream stream;
    message.stringify(stream);
    std::string serializedMessage = stream.str();

    // print command if requested
    if (_printJson)
    {
        std::cout << "Command: " << serializedMessage << std::endl;
    }

    // write message
    _socket.sendMessage(serializedMessage);
    _socket.receiveMessage(response);

    // print reply if requested
    if (_printJson)
    {
        std::cout << "Reply: " << response << std::endl;
    }

    // parse reply data
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var reply;

    try
    {
        reply = parser.parse(response);
    }
    catch(Poco::JSON::JSONException& ex)
    {
        std::cout << ex.message() << std::endl;
        throw std::runtime_error("Error while parsing reply: invalid json");
    }

    return reply;
}

bool JsonConnection::parseReply(const Poco::Dynamic::Var & reply)
{
    bool success;
    std::string reason = "No error info";
    Poco::JSON::Object::Ptr object = reply.extract<Poco::JSON::Object::Ptr>();

    success = object->get("success");
    if (!success)
        reason = object->get("error").toString();

    if (!success)
    {
        throw std::runtime_error("Error: " + reason);
    }

    return success;
}