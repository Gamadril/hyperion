// system includes
#include <stdexcept>
#include <cassert>

// stl includes
#include <iostream>
#include <sstream>
#include <poco-lib/include/Poco/Net/NetException.h>

// hyperion util includes
#include "HyperionConfig.h"
#include "hyperion/ImageProcessorFactory.h"
#include "hyperion/ImageProcessor.h"
#include "hyperion/ColorTransform.h"
#include "utils/JsonTools.h"

#include "Poco/Base64Decoder.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/JSON/Parser.h"

#include "Poco/Net/DNS.h"

// project includes
#include "JsonClientConnection.h"

JsonClientConnection::JsonClientConnection(Poco::Net::WebSocket * socket, Hyperion * hyperion) :
    _socket(socket),
    _imageProcessor(ImageProcessorFactory::getInstance().newImageProcessor()),
    _hyperion(hyperion)
{
}

JsonClientConnection::~JsonClientConnection()
{
    _socket->close();
    delete _imageProcessor;
}

int JsonClientConnection::processRequest()
{
    int flags, size = 0;
    
    // TODO update to use alternate Buffer version in the next poco release
    try {
        size = _socket->receiveFrame(_receiveBuffer, sizeof(_receiveBuffer), flags);
    } catch (Poco::Net::NetException ex) {
        std::cerr << "[ERROR] Could not receive WebSocket frame: " << ex.displayText() << std::endl;
        return -1;
    } catch (Poco::TimeoutException ex) {
        std::cerr << "[ERROR] WebSocket read timeout" << std::endl;
        return -1;
    }

	if ((flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_CLOSE)
	{
		return -1;
	}
    
    if (size > 0)
    {
        if((flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_PING)
        {
            _socket->sendFrame(_receiveBuffer, size, Poco::Net::WebSocket::FRAME_OP_PONG);
        } else
        {
            handleMessage(std::string(_receiveBuffer, size));
        }
    }
	
	return 0;
}

void JsonClientConnection::handleMessage(const std::string & messageString)
{
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var parsedResult;
    Poco::JSON::Object::Ptr message;
	
	//std::cout << messageString << std::endl;

    try
    {
        parsedResult = parser.parse(messageString);
    }
    catch(Poco::JSON::JSONException& ex)
    {
        sendErrorReply("Error while parsing message: " + ex.message());
        return;
    }

    message = parsedResult.extract<Poco::JSON::Object::Ptr>();

    // check specific message
    const std::string command = message->get("command").toString();

    // switch over all possible commands and handle them
    if (command == "color")
        handleColorCommand(message);
    else if (command == "image")
        handleImageCommand(message);
    else if (command == "effect")
        handleEffectCommand(message);
    else if (command == "serverinfo")
        handleServerInfoCommand();
    else if (command == "clear")
        handleClearCommand(message);
    else if (command == "clearall")
        handleClearallCommand();
    else if (command == "transform")
        handleTransformCommand(message);
    else
        handleNotImplemented();
}

void JsonClientConnection::handleColorCommand(const Poco::JSON::Object::Ptr & message)
{
    // extract parameters
    int priority = message->get("priority");
    int duration = message->optValue("duration", -1);

    std::vector<ColorRgb> colorData(_hyperion->getLedCount());
    Poco::JSON::Array::Ptr colors = message->getArray("color");
    unsigned i = 0;
    for (; i < colors->size()/3 && i < _hyperion->getLedCount(); i++)
    {
        colorData[i].red = uint8_t(colors->get(3u*i).extract<int>());
        colorData[i].green = uint8_t(colors->get(3u*i+1u).extract<int>());
        colorData[i].blue = uint8_t(colors->get(3u*i+2u).extract<int>());
    }

    // copy full blocks of led colors
    unsigned size = i;
    while (i + size < _hyperion->getLedCount())
    {
        memcpy(&(colorData[i]), colorData.data(), size * sizeof(ColorRgb));
        i += size;
    }

    // copy remaining block of led colors
    if (i < _hyperion->getLedCount())
    {
        memcpy(&(colorData[i]), colorData.data(), (_hyperion->getLedCount()-i) * sizeof(ColorRgb));
    }

    // set output
    _hyperion->setColors(priority, colorData, duration, true);
	//ColorSetArgs args = {priority, colorData, duration, true};
    //setColorsEvent.notifyAsync(this, args);

    // send reply
    sendSuccessReply();
}

void JsonClientConnection::handleImageCommand(const Poco::JSON::Object::Ptr & message)
{
    // extract parameters
    int priority = message->get("priority");
    int duration = message->optValue("duration", -1);
    const unsigned int width = message->get("imagewidth");
    const unsigned int height = message->get("imageheight");
    std::istringstream istr(message->get("imagedata").toString());
    Poco::Base64Decoder decoder(istr);
 
    // set width and height of the image processor
    _imageProcessor->setSize(width, height);

    // create ImageRgb
    Image<ColorRgb> image(width, height);
    decoder.read((char*)image.memptr(), width*height*3);

    // process the image
    std::vector<ColorRgb> colorData = _imageProcessor->process(image);

    // set output
    _hyperion->setColors(priority, colorData, duration, true);

    // send reply
    sendSuccessReply();
}

void JsonClientConnection::handleEffectCommand(const Poco::JSON::Object::Ptr & message)
{
#ifdef ENABLE_EFFECT_ENGINE
    // extract parameters
    int priority = message->get("priority");
    int duration = message->optValue("duration", -1);
    Poco::JSON::Object::Ptr effect = message->getObject("effect");
    std::string effectName = effect->get("name").toString();
	
	// set output
    if (effect->has("args"))
    {
		Poco::DynamicStruct args = *(effect->getObject("args"));
        _hyperion->setEffect(effectName, args, priority, duration);
    }
    else
    {
        _hyperion->setEffect(effectName, priority, duration);
    }

    // send reply
    sendSuccessReply();
#else
	sendErrorReply("Hyperion was built without effect engine.");
#endif
}

void JsonClientConnection::handleServerInfoCommand()
{
    // create result
    Poco::JSON::Object::Ptr result = new Poco::JSON::Object();
    result->set("success", true);

    Poco::JSON::Object info;
    // add host name for remote clients
    info.set("hostname", Poco::Net::DNS::hostName());

    // collect priority information
    Poco::JSON::Array priorities;
    Poco::Timestamp now;
    std::vector<int> activePriorities = _hyperion->getActivePriorities();
    for (int priority : activePriorities)
    {
        const Hyperion::InputInfo & priorityInfo = _hyperion->getPriorityInfo(priority);
        Poco::JSON::Object item;
        item.set("priority", priority);
        if (priorityInfo.timeoutTime_ms != -1)
        {
            item.set("duration_ms", uint(priorityInfo.timeoutTime_ms - (now.epochMicroseconds() / 1000)));
        }
        priorities.add(item);
    }
    info.set("priorities", priorities);

    // collect transform information
    Poco::JSON::Array transformArray;
    for (const std::string& transformId : _hyperion->getTransformIds())
    {
        const ColorTransform * colorTransform = _hyperion->getTransform(transformId);
        if (colorTransform == nullptr)
        {
            std::cerr << "Incorrect color transform id: " << transformId << std::endl;
            continue;
        }

        Poco::JSON::Object transform;
        transform.set("id", transformId);
        transform.set("saturationGain", colorTransform->_hsvTransform.getSaturationGain());
        transform.set("valueGain", colorTransform->_hsvTransform.getValueGain());

        Poco::JSON::Array threshold;
        threshold.add(colorTransform->_rgbRedTransform.getThreshold());
        threshold.add(colorTransform->_rgbGreenTransform.getThreshold());
        threshold.add(colorTransform->_rgbBlueTransform.getThreshold());
        transform.set("threshold", threshold);

        Poco::JSON::Array gamma;
        gamma.add(colorTransform->_rgbRedTransform.getGamma());
        gamma.add(colorTransform->_rgbGreenTransform.getGamma());
        gamma.add(colorTransform->_rgbBlueTransform.getGamma());
        transform.set("gamma", gamma);

        Poco::JSON::Array blacklevel;
        blacklevel.add(colorTransform->_rgbRedTransform.getBlacklevel());
        blacklevel.add(colorTransform->_rgbGreenTransform.getBlacklevel());
        blacklevel.add(colorTransform->_rgbBlueTransform.getBlacklevel());
        transform.set("blacklevel", blacklevel);

        Poco::JSON::Array whitelevel;
        whitelevel.add(colorTransform->_rgbRedTransform.getWhitelevel());
        whitelevel.add(colorTransform->_rgbGreenTransform.getWhitelevel());
        whitelevel.add(colorTransform->_rgbBlueTransform.getWhitelevel());
        transform.set("whitelevel", whitelevel);
		
        transformArray.add(transform);
    }
    info.set("transform", transformArray);

#ifdef ENABLE_EFFECT_ENGINE	
    // collect effect info
    Poco::JSON::Array effects;
    const std::list<EffectDefinition> & effectsDefinitions = _hyperion->getEffects();
    for (const EffectDefinition & effectDefinition : effectsDefinitions)
    {
        Poco::JSON::Object effect;
        effect.set("name", effectDefinition.name);
        effect.set("script", effectDefinition.script);
		Poco::JSON::Object largs = jsontools::structToJsonObject(effectDefinition.args);
        effect.set("args", largs);
        effects.add(effect);
    }
	info.set("effects", effects);
#endif
	
    result->set("info", info);

    // send the result
    sendMessage(result);
}

void JsonClientConnection::handleClearCommand(const Poco::JSON::Object::Ptr & message)
{
    // extract parameters
    int priority = message->get("priority");

    // clear priority
    _hyperion->clear(priority);

    // send reply
    sendSuccessReply();
}

void JsonClientConnection::handleClearallCommand()
{
    // clear priority
    _hyperion->clearall();

    // send reply
    sendSuccessReply();
}

void JsonClientConnection::handleTransformCommand(const Poco::JSON::Object::Ptr & message)
{
    Poco::JSON::Object::Ptr transform = message->getObject("transform");

    const std::string transformId = transform->optValue("id", _hyperion->getTransformIds().front());
    ColorTransform * colorTransform = _hyperion->getTransform(transformId);
    if (colorTransform == nullptr)
    {
        //sendErrorReply(std::string("Incorrect transform identifier: ") + transformId);
        return;
    }

    if (transform->has("saturationGain"))
    {
        colorTransform->_hsvTransform.setSaturationGain(transform->get("saturationGain").extract<double>());
    }

    if (transform->has("valueGain"))
    {
        colorTransform->_hsvTransform.setValueGain(transform->get("valueGain").extract<double>());
    }

    Poco::JSON::Array::Ptr values;
    if (transform->has("threshold"))
    {
        values = transform->getArray("threshold");
        colorTransform->_rgbRedTransform  .setThreshold(values->get(0));
        colorTransform->_rgbGreenTransform.setThreshold(values->get(1));
        colorTransform->_rgbBlueTransform .setThreshold(values->get(2));
    }

    if (transform->has("gamma"))
    {
        values = transform->getArray("gamma");
        colorTransform->_rgbRedTransform  .setGamma(values->get(0));
        colorTransform->_rgbGreenTransform.setGamma(values->get(1));
        colorTransform->_rgbBlueTransform .setGamma(values->get(2));
    }

    if (transform->has("blacklevel"))
    {
        values = transform->getArray("blacklevel");
        colorTransform->_rgbRedTransform  .setBlacklevel(values->get(0));
        colorTransform->_rgbGreenTransform.setBlacklevel(values->get(1));
        colorTransform->_rgbBlueTransform .setBlacklevel(values->get(2));
    }

    if (transform->has("whitelevel"))
    {
        values = transform->getArray("whitelevel");
        colorTransform->_rgbRedTransform  .setWhitelevel(values->get(0));
        colorTransform->_rgbGreenTransform.setWhitelevel(values->get(1));
        colorTransform->_rgbBlueTransform .setWhitelevel(values->get(2));
    }

    // commit the changes
    _hyperion->transformsUpdated();

    sendSuccessReply();
}

void JsonClientConnection::handleNotImplemented()
{
    sendErrorReply("Command not implemented");
}

void JsonClientConnection::sendMessage(const Poco::JSON::Object::Ptr & message)
{
    // serialize message
    std::ostringstream stream;
    message->stringify(stream);
    std::string serializedReply = stream.str();
	
	//std::cout << "REPLY: " << serializedReply << std::endl;

    _socket->sendFrame(serializedReply.c_str(), (int) serializedReply.size());
}

void JsonClientConnection::sendSuccessReply()
{
    // create reply
    Poco::JSON::Object::Ptr reply = new Poco::JSON::Object();
    reply->set("success", true);

    // send reply
    sendMessage(reply);
}

void JsonClientConnection::sendErrorReply(const std::string & error)
{
    // create reply
    Poco::JSON::Object::Ptr reply = new Poco::JSON::Object();
    reply->set("success", false);
    reply->set("error", error);

    // send reply
    sendMessage(reply);
}
/*
bool JsonClientConnection::checkJson(const Poco::JSON::Object::Ptr & message, const QString & schemaResource, std::string & errorMessage)
{
	// read the json schema from the resource
	QResource schemaData(schemaResource);
	assert(schemaData.isValid());
	Json::Reader jsonReader;
	Json::Value schemaJson;
	if (!jsonReader.parse(reinterpret_cast<const char *>(schemaData.data()), reinterpret_cast<const char *>(schemaData.data()) + schemaData.size(), schemaJson, false))
	{
		throw std::runtime_error("Schema error: " + jsonReader.getFormattedErrorMessages())	;
	}

	// create schema checker
	JsonSchemaChecker schema;
	schema.setSchema(schemaJson);

	// check the message
	if (!schema.validate(message))
	{
		const std::list<std::string> & errors = schema.getMessages();
		std::stringstream ss;
		ss << "{";
		foreach (const std::string & error, errors) {
			ss << error << " ";
		}
		ss << "}";
		errorMessage = ss.str();
		return false;
	}

	return true;
}
*/
